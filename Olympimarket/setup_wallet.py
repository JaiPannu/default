import json
import time
from solders.keypair import Keypair
from solana.rpc.api import Client
from solders.pubkey import Pubkey

# CONFIGURATION
WALLET_FILENAME = "hackathon-wallet.json"
LOCAL_RPC_URL = "http://127.0.0.1:8899"
AIRDROP_AMOUNT = 10 * 10**9  # 10 SOL (in lamports)

def create_and_fund_wallet():
    print("--- 🛠️  SOLANA WALLET SETUP 🛠️ ---")

    # 1. GENERATE NEW KEYPAIR
    # This creates a brand new random wallet
    new_keypair = Keypair()
    
    # 2. SAVE TO FILE
    # We save the raw bytes (private key) as a JSON array
    # This matches the format expected by 'solana-keygen' and your bridge script
    with open(WALLET_FILENAME, 'w') as f:
        json.dump(new_keypair.to_bytes_array(), f)
    
    print(f"✅ Wallet generated and saved to '{WALLET_FILENAME}'")
    print(f"🔑 Public Key: {new_keypair.pubkey()}")
    print("----------------------------------------")

    # 3. FUND THE WALLET (AIRDROP)
    # Without this, your robot cannot pay the 0.000005 SOL fee to record the score.
    print(f"💸 Attempting to airdrop 10 SOL to {new_keypair.pubkey()}...")
    
    client = Client(LOCAL_RPC_URL)
    
    try:
        # Check if Validator is actually running
        if not client.is_connected():
            print("❌ ERROR: Could not connect to Local Validator.")
            print("   -> Run 'solana-test-validator' in a separate terminal first!")
            return

        # Request Airdrop
        # Note: 'confirm_transaction' is often needed immediately after airdrop on localnet
        airdrop_sig = client.request_airdrop(new_keypair.pubkey(), AIRDROP_AMOUNT).value
        
        print(f"   -> Airdrop Sent! Signature: {airdrop_sig}")
        print("   -> Waiting for confirmation...")
        
        # Wait for the money to actually arrive
        client.confirm_transaction(airdrop_sig)
        
        # 4. VERIFY BALANCE
        balance = client.get_balance(new_keypair.pubkey()).value
        print(f"✅ SUCCESS! New Balance: {balance / 10**9} SOL")
        print("----------------------------------------")
        print("🚀 YOU ARE READY. Run 'bridge.py' now.")

    except Exception as e:
        print(f"❌ Airdrop Failed: {e}")
        print("   -> Ensure 'solana-test-validator' is running.")

if __name__ == "__main__":
    create_and_fund_wallet()