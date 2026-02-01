import serial
import time
import json
import random
import os
from solders.keypair import Keypair
from solana.rpc.api import Client

# --- CONFIG ---
SIMULATION_MODE = True  # <--- SET TO TRUE TO TEST WITHOUT ROBOT
ARDUINO_PORT = "COM3"   # Ignored if SIMULATION_MODE is True
BAUD_RATE = 115200
STATE_FILE = "race_state.json"
WALLET_PATH = "hackathon-wallet.json"

# --- INIT ---
print(f"🚀 Bridge Starting... (Simulation Mode: {SIMULATION_MODE})")

# Reset State File
initial_state = {"status": "WAITING", "time": 0, "score": 0}
with open(STATE_FILE, 'w') as f:
    json.dump(initial_state, f)

# Connect to Serial (only if not simulating)
ser = None
if not SIMULATION_MODE:
    try:
        ser = serial.Serial(ARDUINO_PORT, BAUD_RATE, timeout=1)
        print(f"✅ Connected to {ARDUINO_PORT}")
    except:
        print("❌ ERROR: Arduino not found. Switch SIMULATION_MODE = True to test.")

def update_ui(status, time_val, score_val):
    """Writes to JSON for Streamlit"""
    data = {
        "status": status,
        "time": round(time_val, 2),
        "score": score_val,
        "last_update": time.time()
    }
    # Atomic write to prevent file corruption
    temp_file = STATE_FILE + ".tmp"
    with open(temp_file, 'w') as f:
        json.dump(data, f)
    os.replace(temp_file, STATE_FILE)

# --- SIMULATION VARIABLES ---
sim_start_time = 0
sim_running = False

# --- MAIN LOOP ---
while True:
    try:
        # ==========================================
        # 👻 GHOST MODE (SIMULATION)
        # ==========================================
        if SIMULATION_MODE:
            # Randomly start a race every 10 seconds if not running
            if not sim_running and random.random() < 0.05:
                print("👻 SIMULATION: Race Started!")
                sim_running = True
                sim_start_time = time.time()
            
            if sim_running:
                elapsed = time.time() - sim_start_time
                
                # 1. Simulate Telemetry (Running)
                if elapsed < 5.0: # Race lasts 5 seconds
                    # Fake telemetry update
                    update_ui("RACING", elapsed, 0)
                    print(f"👻 Simulating Run: {elapsed:.2f}s")
                
                # 2. Simulate Finish Line
                else:
                    final_score = random.randint(30, 50)
                    print(f"🏆 SIMULATION FINISH! Time: {elapsed:.2f}s, Score: {final_score}")
                    update_ui("FINISHED", elapsed, final_score)
                    
                    # Pause before resetting
                    time.sleep(5)
                    sim_running = False
                    update_ui("WAITING", 0, 0)

            time.sleep(0.2) # Update speed

        # ==========================================
        # 🤖 REAL ROBOT MODE
        # ==========================================
        elif ser and ser.in_waiting > 0:
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            
            if line.startswith("LOG:"):
                # LOG:TIME,L,R,DIST,PWM
                parts = line.split(",")
                run_time = int(parts[0]) / 1000.0
                update_ui("RACING", run_time, 0)

            elif line.startswith("SOLANA_RECORD:"):
                parts = line.split(":")
                score = int(parts[1])
                final_time = int(parts[2]) / 1000.0
                update_ui("FINISHED", final_time, score)
                # Add blockchain send logic here

    except Exception as e:
        print(f"Error: {e}")
        time.sleep(1)