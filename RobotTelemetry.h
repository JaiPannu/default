#define DEBUG_MODE true       // Set to false for competition run
#define TELEMETRY_RATE 100    // How often to log in ms (avoid flooding Serial)

// Pin Definitions (Adjust to match your wiring)
const int ENC_LEFT_PIN = 2;   // IR Sensor hacked as encoder
const int ENC_RIGHT_PIN = 3;  // IR Sensor hacked as encoder
const int TRIG_PIN = 4;       // Ultrasonic Trig
const int ECHO_PIN = 5;       // Ultrasonic Echo
// Assumes standard TCS3200 or similar for Color Sensor

// Globals for Tracking
volatile long leftTicks = 0;
volatile long rightTicks = 0;
unsigned long lastTelemetryTime = 0;

void setupTracking() {
  Serial.begin(115200); // Use 115200 for faster data transfer
  
  pinMode(ENC_LEFT_PIN, INPUT);
  pinMode(ENC_RIGHT_PIN, INPUT);
  
  // Attach interrupts for "hacked" IR encoders
  attachInterrupt(digitalPinToInterrupt(ENC_LEFT_PIN), countLeft, RISING);
  attachInterrupt(digitalPinToInterrupt(ENC_RIGHT_PIN), countRight, RISING);
  
  // Ultrasonic setup
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

// Interrupt Service Routines (Keep these short!)
void countLeft() { leftTicks++; }
void countRight() { rightTicks++; }

// Call this function inside your main loop()
void logTelemetry() {
  if (!DEBUG_MODE) return;

  if (millis() - lastTelemetryTime > TELEMETRY_RATE) {
    lastTelemetryTime = millis();
    
    // 1. Read Ultrasonic
    long duration, distance;
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    duration = pulseIn(ECHO_PIN, HIGH);
    distance = duration * 0.034 / 2;

    // 2. Format Output for Serial Plotter / Monitor
    // Format: "TIME | L_TICKS | R_TICKS | DIST_CM | STATE"
    Serial.print("LOG:");
    Serial.print(millis());
    Serial.print(",");
    Serial.print(leftTicks);
    Serial.print(",");
    Serial.print(rightTicks);
    Serial.print(",");
    Serial.print(distance);
    Serial.print(",");
    // Add logic here to print current Color Sensor state if needed
    Serial.println(); 
  }
}

// The Critical Handshake for your Python Bridge
void sendSolanaHandshake(int finalScore, long runTimeMs) {
  // Ensure we don't have dangling characters
  Serial.flush(); 
  
  // Protocol: SOLANA_RECORD:<SCORE>:<TIME>
  Serial.print("SOLANA_RECORD:");
  Serial.print(finalScore);
  Serial.print(":");
  Serial.println(runTimeMs);
}