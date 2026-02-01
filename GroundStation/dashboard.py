import streamlit as st
import json
import time
import pandas as pd
import plotly.express as px

st.set_page_config(page_title="Biathlon Prediction Market", layout="wide")

# --- LOAD STATE ---
def load_state():
    try:
        with open("race_state.json", "r") as f:
            return json.load(f)
    except:
        return {"status": "DISCONNECTED", "logs": []}

state = load_state()

# --- HEADER ---
st.title("🤖 UTRA Biathlon: The Prediction Market")
st.markdown("### Status: " + f"**{state['status']}**")

# --- PHASE 1: THE BETTING FLOOR (Offline) ---
if state['status'] == "DISCONNECTED" or state['status'] == "RUNNING_OFFLINE":
    st.info("🔌 ROBOT IS OFFLINE (ON TRACK). PLACE YOUR BETS!")
    
    col1, col2, col3 = st.columns(3)
    with col1:
        st.metric("Current Odds", "2.5x")
        if st.button("Bet: PERFECT RUN (No Obstacles)"):
            st.toast("Bet Placed: Perfect Run")
    with col2:
        st.metric("Current Odds", "1.2x")
        if st.button("Bet: SAFETY FINISH (Score > 20)"):
            st.toast("Bet Placed: Safety Finish")
    with col3:
        st.metric("Current Odds", "5.0x")
        if st.button("Bet: CRASH / DNF"):
            st.toast("Bet Placed: Crash")

    st.image("https://media.giphy.com/media/l0HlHFRbmaZtBRhXG/giphy.gif", caption="Waiting for connection at Re-upload Point...")

# --- PHASE 2: THE REVEAL (Data Burst) ---
elif state['status'] == "SYNCED":
    st.success("DATA BURST RECEIVED! VERIFYING ON SOLANA...")
    
    # Process the Logs
    logs = state['logs'] # List of {timestamp, type, value}
    df = pd.DataFrame(logs)
    
    # 1. The "Lie Detector" Graph
    # Plot sensor values over time to prove the robot actually ran
    if not df.empty:
        fig = px.line(df, x="timestamp", y="value", title="Robot Telemetry Replay")
        st.plotly_chart(fig, use_container_width=True)

    # 2. Event Timeline
    st.subheader("Race Replay")
    for log in logs:
        if log['type'] == 2:
            st.error(f"⏱ {log['timestamp']}ms: OBSTACLE HIT! (-5 Pts)")
        elif log['type'] == 3:
            st.info(f"⏱ {log['timestamp']}ms: Box Picked Up")
        elif log['type'] == 5:
            st.balloons()
            st.success(f"⏱ {log['timestamp']}ms: TARGET SHOT! (+{log['value']} Pts)")

    # 3. Blockchain Proof
    if "tx_signature" in state:
        st.markdown("---")
        st.markdown(f"**⛓️ Blockchain Proof:** [`{state['tx_signature']}`](https://explorer.solana.com/tx/{state['tx_signature']}?cluster=custom)")

# --- AUTO REFRESH ---
time.sleep(2)
st.rerun()