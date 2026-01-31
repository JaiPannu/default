// ================= SECTION 1: TARGET SHOOTING =================

// ================= MOTOR DRIVER PINS (SAME) =================
const int ENA = 5;
const int IN1 = 8;
const int IN2 = 9;

const int ENB = 6;
const int IN3 = 10;
const int IN4 = 11;

// ================= IR SENSORS (SAME) =================
const int IR_LEFT  = A0;
const int IR_RIGHT = A1;

// ================= ULTRASONIC SENSOR (SAME) =================
const int TRIG_PIN = 2;
const int ECHO_PIN = 3;

// ================= SERVO MOTORS (SAME) =================
#include <Servo.h>
Servo armServo;
Servo clawServo;
const int ARM_PIN  = 12;
const int CLAW_PIN = 13;

// ================= SECTION 1 PARAMETERS =================
// Higher speed needed for the ramp!
int cruiseSpeed = 180;  // Normal driving
int rampSpeed = 255;    // Max power for the shot approach

// IR Thresholds
// Adjust this! Usually: White < 200, Black > 600
int blackThreshold = 600; 

// State tracking
bool targetReached = false;

// ================= SETUP =================
void setup() {
  Serial.begin(9600);

  // Motor pins
  pinMode(ENA, OUTPUT); pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT); pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);

  // Sensors
  pinMode(IR_LEFT, INPUT);
  pinMode(IR_RIGHT, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Servos
  armServo.attach(ARM_PIN);
  clawServo.attach(CLAW_PIN);

  // STARTING POSITION
  // Hold the ball gently (claw slightly closed)
  armUp();      
  clawHoldBall(); 
}

// ================= MAIN LOOP =================
void loop() {
  
  // If we already shot, do nothing (wait for reset)
  if (targetReached) {
    stopMotors();
    return;
  }

  // Read IR Sensors
  int leftVal = analogRead(IR_LEFT);
  int rightVal = analogRead(IR_RIGHT);

  // ================= LOGIC: FIND THE BLACK ZONE =================
  
  // CASE 1: BOTH sensors see BLACK (We are in the Target Zone!)
  if (leftVal > blackThreshold && rightVal > blackThreshold) {
    stopMotors();
    delay(500); // Stabilize
    shootBall(); // FIRE!
    targetReached = true;
  }
  
  // CASE 2: LINE FOLLOWING (Green Path)
  // Assuming Green reflects less than White but more than Black
  // You might need to tune 'blackThreshold' to distinguish Green vs Black
  else {
    followLine(leftVal, rightVal);
  }
}

// ================= LINE FOLLOWING =================
void followLine(int left, int right) {
  // Simple "Bang-Bang" control for the ramp
  
  // If centered (both see "path" or "white" depending on your tape)
  // Modify logic based on if line is DARK or LIGHT
  // ASSUMING: Line is DARK, Floor is LIGHT
  
  if (left < blackThreshold && right < blackThreshold) {
    moveForward(cruiseSpeed);
  }
  else if (left > blackThreshold) { 
    // Left sees line -> Turn Left
    turnLeft();
  }
  else if (right > blackThreshold) {
    // Right sees line -> Turn Right
    turnRight();
  }
}

// ================= THE "BIATHLON" SHOT =================
void shootBall() {
  // 1. Back up to get runway space
  moveBackward();
  delay(800); 
  stopMotors();
  delay(200);

  // 2. Drive Forward at MAX SPEED (Kinetic Energy)
  armDown(); // Lower arm slightly for better center of mass
  moveForward(255); // MAX SPEED
  delay(600); // Adjust time based on distance to wall

  // 3. THE SNAP: Hard Stop + Arm Swing
  // Stop motors abruptly (Active Braking)
  digitalWrite(ENA, HIGH); 
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW); // Brake mode on some drivers
  digitalWrite(ENB, HIGH); 
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  
  // INSTANTLY swing arm up to fling ball
  armThrow(); 
  
  delay(1000); // Celebrate
}

// ================= MOTOR FUNCTIONS =================
void moveForward(int speed) {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  analogWrite(ENA, speed); analogWrite(ENB, speed);
}

void moveBackward() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  analogWrite(ENA, cruiseSpeed); analogWrite(ENB, cruiseSpeed);
}

void turnLeft() {
  analogWrite(ENA, 80);  // Slow left
  analogWrite(ENB, 180); // Fast right
}

void turnRight() {
  analogWrite(ENA, 180); // Fast left
  analogWrite(ENB, 80);  // Slow right
}

void stopMotors() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}

// ================= SERVO MACROS =================
void armUp() { armServo.write(60); }     // Carrying position
void armDown() { armServo.write(100); }  // Ready to scoop/launch
void armThrow() { armServo.write(20); }  // Fling position (fast movement)

void clawOpen() { clawServo.write(90); }
void clawClose() { clawServo.write(30); }     // Tight grip for box
void clawHoldBall() { clawServo.write(60); }  // Loose grip (cup) for ball