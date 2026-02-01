import streamlit as st
import json
import time
import pandas as pd

# --- PAGE CONFIG ---
st.set_page_config(page_title="Olympimarket", layout="wide", page_icon="🤖")
st.title("🤖 Olympimarket: The Live Robot Economy")

# --- SIDEBAR (USER WALLET) ---
if 'balance' not in st.session_state:
    st.session_state.balance = 1000
    st.session_state.my_bets = []

st.sidebar.header("💳 Judge's Wallet")
st.sidebar.metric("Your Balance", f"${st.session_state.balance} FAN")
st.sidebar.markdown("---")
st.sidebar.subheader("Your Active Bets")
if st.session_state.my_bets:
    for bet in st.session_state.my_bets:
        st.sidebar.info(f"{bet['type']}: ${bet['amount']}")
else:
    st.sidebar.text("No active bets.")

# --- LIVE DATA READER ---
def get_robot_state():
    try:
        with open("race_state.json", "r") as f:
            return json.load(f)
    except:
        return {"status": "OFFLINE", "time": 0, "score": 0}

state = get_robot_state()

# --- MAIN DASHBOARD ---
# Top Row: Stats
kpi1, kpi2, kpi3 = st.columns(3)
kpi1.metric("Robot Status", state['status'], delta_color="inverse")
kpi2.metric("Race Timer", f"{state['time']} s")
kpi3.metric("Projected Score", f"{state['score']} pts")

# Middle Row: Betting Floor
st.markdown("### 🎲 Live Prediction Market")
col1, col2 = st.columns(2)

with col1:
    st.info("📉 **Short Position: The Skeptic**")
    st.caption("Bet that the robot CRASHES or takes > 45s.")
    if st.button("Bet $100 on FAIL"):
        st.session_state.balance -= 100
        st.session_state.my_bets.append({"type": "FAIL", "amount": 100})
        st.toast("Bet Placed: $100 on FAILURE")

with col2:
    st.success("📈 **Long Position: The Believer**")
    st.caption("Bet that the robot SUCCEEDS (Time < 45s).")
    if st.button("Bet $100 on SUCCESS"):
        st.session_state.balance -= 100
        st.session_state.my_bets.append({"type": "SUCCESS", "amount": 100})
        st.toast("Bet Placed: $100 on SUCCESS")

# --- SETTLEMENT LOGIC ---
if state['status'] == "FINISHED":
    st.balloons()
    st.success(f"RACE COMPLETE! Final Time: {state['time']}s")
    # Simple settlement (demo only)
    if state['time'] < 45000: # 45 seconds
        st.markdown("## ✅ RESULT: SUCCESS")
    else:
        st.markdown("## ❌ RESULT: FAIL")

# Auto-refresh every 1 second
time.sleep(1)
st.rerun()