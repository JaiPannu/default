from flask import Flask, render_template, jsonify, request, session, render_template_string
import json
import os
from datetime import datetime, timedelta
import uuid
import random

app = Flask(__name__)
app.secret_key = 'olympimarket_secret_key_2026'

# Data file paths
RACE_STATE_FILE = 'race_state.json'
BETS_FILE = 'bets.json'

# DEBUG MODE - Set to True for demo
DEBUG_MODE = True
DEMO_RACES = [
    {"id": 1, "name": "Biathlon Challenge", "robot": "BiathlonBot", "status": "READY"},
    {"id": 2, "name": "Speed Trial", "robot": "FastBot", "status": "RUNNING"},
    {"id": 3, "name": "Obstacle Course", "robot": "NavigatorBot", "status": "FINISHED"},
    {"id": 4, "name": "Endurance Test", "robot": "PowerBot", "status": "READY"},
]
CURRENT_RACE_ID = 2  # Which race to display on dashboard

def load_race_state():
    """Load current robot race state"""
    try:
        with open(RACE_STATE_FILE, 'r') as f:
            return json.load(f)
    except:
        if DEBUG_MODE:
            return get_debug_race_state()
        return {"status": "OFFLINE", "time": 0, "score": 0, "races": []}

def get_debug_race_state():
    """Generate demo race state based on current race ID"""
    race = next((r for r in DEMO_RACES if r["id"] == CURRENT_RACE_ID), DEMO_RACES[0])
    
    # Simulate different states for different races
    if race["status"] == "RUNNING":
        time_elapsed = random.randint(10, 40)
        score = time_elapsed * 12 + random.randint(-50, 100)
    elif race["status"] == "FINISHED":
        time_elapsed = random.choice([32, 38, 45, 52])  # Include some failures
        score = (time_elapsed * 12) if time_elapsed < 45 else (time_elapsed * 8)
    else:
        time_elapsed = 0
        score = 0
    
    return {
        "status": race["status"],
        "time": time_elapsed,
        "score": score,
        "race_id": race["id"],
        "race_name": race["name"],
        "robot": race["robot"],
        "races": DEMO_RACES
    }

def load_bets():
    """Load all bets from file"""
    try:
        with open(BETS_FILE, 'r') as f:
            return json.load(f)
    except:
        return {}

def save_bets(bets_data):
    """Save bets to file"""
    with open(BETS_FILE, 'w') as f:
        json.dump(bets_data, f, indent=2)

def get_user_id():
    """Get or create user session ID"""
    if 'user_id' not in session:
        session['user_id'] = str(uuid.uuid4())
    return session['user_id']

def get_market_data():
    """Calculate market data (odds, total volume, etc.)"""
    state = load_race_state()
    bets = load_bets()
    
    if DEBUG_MODE:
        # Generate demo market data for current race
        race_id = state.get('race_id', 1)
        success_bets = random.randint(500, 2000)
        fail_bets = random.randint(300, 1500)
    else:
        success_bets = sum(b['amount'] for b in bets.values() if b.get('position') == 'SUCCESS')
        fail_bets = sum(b['amount'] for b in bets.values() if b.get('position') == 'FAIL')
    
    total = success_bets + fail_bets
    if total == 0:
        success_odds = 50
        fail_odds = 50
    else:
        success_odds = (success_bets / total) * 100
        fail_odds = (fail_bets / total) * 100
    
    return {
        'success_odds': round(success_odds, 1),
        'fail_odds': round(fail_odds, 1),
        'success_volume': success_bets,
        'fail_volume': fail_bets,
        'total_volume': total,
        'participants': len(bets) if not DEBUG_MODE else random.randint(5, 50)
    }

@app.route('/')
def index():
    """Main dashboard page"""
    state = load_race_state()
    market_data = get_market_data()
    user_id = get_user_id()
    
    # Get user's bets
    bets = load_bets()
    user_bets = bets.get(user_id, {})
    user_balance = user_bets.get('balance', 1000)
    user_positions = user_bets.get('positions', [])
    
    # Add debug flag to template
    return render_template('index.html', 
                         state=state, 
                         market_data=market_data,
                         user_balance=user_balance,
                         user_positions=user_positions,
                         debug_mode=DEBUG_MODE,
                         demo_races=DEMO_RACES,
                         current_race_id=CURRENT_RACE_ID)

@app.route('/api/market-data')
def api_market_data():
    """API endpoint for market data"""
    market_data = get_market_data()
    state = load_race_state()
    return jsonify({
        'status': state.get('status', 'OFFLINE'),
        'time': state.get('time', 0),
        'score': state.get('score', 0),
        'market': market_data,
        'timestamp': datetime.now().isoformat()
    })

@app.route('/api/place-bet', methods=['POST'])
def api_place_bet():
    """Place a bet on a market"""
    data = request.json
    user_id = get_user_id()
    position = data.get('position')  # 'SUCCESS' or 'FAIL'
    amount = float(data.get('amount', 0))
    
    if amount <= 0:
        return jsonify({'success': False, 'error': 'Invalid amount'}), 400
    
    bets = load_bets()
    if user_id not in bets:
        bets[user_id] = {'balance': 1000, 'positions': []}
    
    user_bets = bets[user_id]
    if user_bets['balance'] < amount:
        return jsonify({'success': False, 'error': 'Insufficient balance'}), 400
    
    # Deduct from balance and add position
    user_bets['balance'] -= amount
    user_bets['positions'].append({
        'id': str(uuid.uuid4()),
        'position': position,
        'amount': amount,
        'timestamp': datetime.now().isoformat(),
        'status': 'OPEN'
    })
    
    save_bets(bets)
    market_data = get_market_data()
    
    return jsonify({
        'success': True,
        'new_balance': user_bets['balance'],
        'market_data': market_data
    })

@app.route('/api/user-positions')
def api_user_positions():
    """Get user's current positions"""
    user_id = get_user_id()
    bets = load_bets()
    
    if user_id not in bets:
        return jsonify({'balance': 1000, 'positions': []})
    
    return jsonify(bets[user_id])

@app.route('/history')
def history():
    """Betting history page"""
    user_id = get_user_id()
    bets = load_bets()
    user_bets = bets.get(user_id, {})
    
    return render_template('history.html', positions=user_bets.get('positions', []))

# ===== DEBUG ENDPOINTS =====

@app.route('/api/debug/races')
def api_debug_races():
    """Get all demo races"""
    if not DEBUG_MODE:
        return jsonify({'error': 'Debug mode disabled'}), 403
    return jsonify({
        'races': DEMO_RACES,
        'current_race_id': CURRENT_RACE_ID,
        'debug_mode': DEBUG_MODE
    })

@app.route('/api/debug/race/<int:race_id>')
def api_debug_set_race(race_id):
    """Switch to a different demo race"""
    global CURRENT_RACE_ID
    if not DEBUG_MODE:
        return jsonify({'error': 'Debug mode disabled'}), 403
    
    if not any(r['id'] == race_id for r in DEMO_RACES):
        return jsonify({'error': 'Race not found'}), 404
    
    CURRENT_RACE_ID = race_id
    state = get_debug_race_state()
    market_data = get_market_data()
    
    return jsonify({
        'success': True,
        'current_race_id': CURRENT_RACE_ID,
        'state': state,
        'market_data': market_data
    })

@app.route('/api/debug/simulate')
def api_debug_simulate():
    """Simulate race progression for current race"""
    if not DEBUG_MODE:
        return jsonify({'error': 'Debug mode disabled'}), 403
    
    race = next((r for r in DEMO_RACES if r["id"] == CURRENT_RACE_ID), DEMO_RACES[0])
    
    # Cycle through states
    states = ["READY", "RUNNING", "FINISHED"]
    current_state = race["status"]
    next_state = states[(states.index(current_state) + 1) % len(states)]
    
    # Update race state
    for r in DEMO_RACES:
        if r["id"] == CURRENT_RACE_ID:
            r["status"] = next_state
            break
    
    state = get_debug_race_state()
    market_data = get_market_data()
    
    return jsonify({
        'success': True,
        'race_id': CURRENT_RACE_ID,
        'state': state,
        'market_data': market_data,
        'message': f'Race {CURRENT_RACE_ID} transitioned to {next_state}'
    })

@app.route('/api/debug/reset')
def api_debug_reset():
    """Reset all demo data and races"""
    if not DEBUG_MODE:
        return jsonify({'error': 'Debug mode disabled'}), 403
    
    global CURRENT_RACE_ID
    
    # Reset races
    for i, race in enumerate(DEMO_RACES):
        race['status'] = ['READY', 'RUNNING', 'FINISHED', 'READY'][i % 3]
    
    CURRENT_RACE_ID = 2
    
    # Create fresh demo bets
    create_demo_bets()
    
    return jsonify({
        'success': True,
        'message': 'All demo data reset',
        'races': DEMO_RACES
    })

@app.route('/api/debug/populate-bets')
def api_debug_populate_bets():
    """Generate random demo bets for testing"""
    if not DEBUG_MODE:
        return jsonify({'error': 'Debug mode disabled'}), 403
    
    create_demo_bets()
    bets = load_bets()
    
    return jsonify({
        'success': True,
        'message': f'Generated demo bets for {len(bets)} users',
        'user_count': len(bets)
    })

def create_demo_bets():
    """Create demo betting data"""
    demo_users = {}
    
    # Create 10 demo users with various bet positions
    for i in range(10):
        user_id = f"demo-user-{i}"
        balance = random.randint(100, 2000)
        
        positions = []
        # Each user has 1-3 positions
        for j in range(random.randint(1, 3)):
            positions.append({
                'id': str(uuid.uuid4()),
                'position': random.choice(['SUCCESS', 'FAIL']),
                'amount': random.randint(50, 500),
                'timestamp': (datetime.now() - timedelta(minutes=random.randint(0, 60))).isoformat(),
                'status': random.choice(['OPEN', 'WON', 'LOST'])
            })
        
        demo_users[user_id] = {
            'balance': balance,
            'positions': positions
        }
    
    save_bets(demo_users)

@app.route('/debug')
def debug_dashboard():
    """Debug control panel for demo"""
    if not DEBUG_MODE:
        return "Debug mode is disabled", 404
    
    html = '''
    <!DOCTYPE html>
    <html>
    <head>
        <title>Olympimarket - Debug Panel</title>
        <style>
            body { font-family: Arial; background: #0f172a; color: #f1f5f9; padding: 20px; }
            .container { max-width: 800px; margin: 0 auto; }
            h1 { color: #1e40af; }
            .section { background: #1a2332; border: 1px solid #334155; border-radius: 8px; padding: 20px; margin: 20px 0; }
            .race-list { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 10px; }
            .race-card { background: #2d3e52; border: 2px solid #334155; padding: 15px; border-radius: 6px; cursor: pointer; }
            .race-card:hover { border-color: #1e40af; }
            .race-card.active { border-color: #10b981; background: rgba(16,185,129,0.1); }
            button { background: #1e40af; color: white; border: none; padding: 10px 20px; border-radius: 6px; cursor: pointer; font-size: 14px; margin: 5px; }
            button:hover { background: #1e3a8a; }
            .status { display: inline-block; padding: 5px 10px; border-radius: 4px; font-size: 12px; font-weight: bold; }
            .ready { background: #cbd5e1; color: #0f172a; }
            .running { background: #10b981; color: white; }
            .finished { background: #f59e0b; color: white; }
            .info { background: #2d3e52; padding: 10px; margin: 10px 0; border-left: 3px solid #1e40af; }
            .button-group { margin: 20px 0; }
        </style>
    </head>
    <body>
        <div class="container">
            <h1>🤖 Olympimarket Debug Panel</h1>
            <p>Control demo races and simulate robot behavior for testing.</p>
            
            <div class="section">
                <h2>Current Race</h2>
                <div id="current-race" class="info">Loading...</div>
            </div>
            
            <div class="section">
                <h2>Select Demo Race</h2>
                <div class="race-list" id="race-list"></div>
            </div>
            
            <div class="section">
                <h2>Controls</h2>
                <div class="button-group">
                    <button onclick="simulateRace()">📊 Simulate Race Progression</button>
                    <button onclick="generateBets()">💰 Generate Random Bets</button>
                    <button onclick="resetDemo()">🔄 Reset All Demo Data</button>
                </div>
            </div>
            
            <div class="section">
                <h2>Status</h2>
                <div id="status" class="info">Ready</div>
            </div>
        </div>
        
        <script>
            async function loadRaces() {
                const res = await fetch('/api/debug/races');
                const data = await res.json();
                displayRaces(data.races, data.current_race_id);
                updateCurrentRace(data.current_race_id);
            }
            
            function displayRaces(races, currentId) {
                const list = document.getElementById('race-list');
                list.innerHTML = races.map(race => `
                    <div class="race-card ${race.id === currentId ? 'active' : ''}" onclick="switchRace(${race.id})">
                        <strong>${race.name}</strong>
                        <div style="margin-top: 8px;">
                            <span class="status ${race.status.toLowerCase()}">${race.status}</span>
                        </div>
                        <small style="color: #cbd5e1;">🤖 ${race.robot}</small>
                    </div>
                `).join('');
            }
            
            async function switchRace(raceId) {
                setStatus('Switching race...');
                const res = await fetch(`/api/debug/race/${raceId}`);
                const data = await res.json();
                updateCurrentRace(raceId);
                loadRaces();
                setStatus(`✓ Switched to ${data.state.race_name}`);
            }
            
            async function updateCurrentRace(raceId) {
                const res = await fetch('/api/market-data');
                const data = await res.json();
                const stateStr = `
                    <strong>${data.status}</strong><br>
                    Status: ${data.status}<br>
                    Time: ${data.time}s<br>
                    Score: ${data.score} pts<br>
                    <br>
                    <strong>Market Odds:</strong><br>
                    YES: ${data.market.success_odds}%<br>
                    NO: ${data.market.fail_odds}%<br>
                    Volume: $${data.market.total_volume}
                `;
                document.getElementById('current-race').innerHTML = stateStr;
            }
            
            async function simulateRace() {
                setStatus('Simulating race...');
                const res = await fetch('/api/debug/simulate');
                const data = await res.json();
                loadRaces();
                updateCurrentRace(data.race_id);
                setStatus(`✓ ${data.message}`);
            }
            
            async function generateBets() {
                setStatus('Generating bets...');
                const res = await fetch('/api/debug/populate-bets');
                const data = await res.json();
                setStatus(`✓ ${data.message}`);
            }
            
            async function resetDemo() {
                if (!confirm('Reset all demo data?')) return;
                setStatus('Resetting...');
                const res = await fetch('/api/debug/reset');
                const data = await res.json();
                loadRaces();
                updateCurrentRace(2);
                setStatus(`✓ ${data.message}`);
            }
            
            function setStatus(msg) {
                document.getElementById('status').textContent = msg;
            }
            
            // Load on startup
            loadRaces();
            setInterval(updateCurrentRace, 2000);
        </script>
    </body>
    </html>
    '''
    return render_template_string(html)

if __name__ == '__main__':
    # Initialize demo bets if in debug mode
    if DEBUG_MODE:
        if not os.path.exists(BETS_FILE) or os.path.getsize(BETS_FILE) < 5:
            create_demo_bets()
    
    app.run(debug=True, port=5000)
