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

// ================= COLOR SENSOR (TCS3200) =================
const int s0  = 4;
const int s1  = 7;
const int s2  = A2;
const int s3  = A3;
const int out = A4;

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

// State tracking
bool targetReached = false;
bool boxPickedUp = false;
String lastDetectedColor = "null";  // Track the last detected color

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

  // Color sensor pins
  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
  pinMode(s3, OUTPUT);
  pinMode(out, INPUT);

  digitalWrite(s0, HIGH);
  digitalWrite(s1, HIGH);

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

  // Detect current color in front
  String currentColor = detectColour();

  // PRIORITY 1: RED ZONE DETECTED - SHOOT!
  if (currentColor == "red") {
    stopMotors();
    delay(500); // Stabilize
    shootBall(); // FIRE!
    targetReached = true;
  }
  
  // PRIORITY 2: COLOR TRANSITIONS (BLUE DETECTION)
  // If we transition from GREEN to BLUE, pick up box
  else if (lastDetectedColor == "green" && currentColor == "blue" && !boxPickedUp) {
    pickUpBox();
  }
  
  // If we're carrying box and detect BLUE again, drop it
  else if (currentColor == "blue" && boxPickedUp) {
    dropBox();
  }
  
  // PRIORITY 3: FOLLOW THE PATH (GREEN or BLACK)
  else {
    followPath(currentColor);
  }
  
  // Update last detected color
  if (currentColor != "null") {
    lastDetectedColor = currentColor;
  }
}

// ================= PATH FOLLOWING =================
void followPath(String currentColor) {
  // GREEN or BLACK PATH: Move forward along the line
  if (currentColor == "green" || currentColor == "black") {
    moveForward(cruiseSpeed);
    delay(50);
  }
  
  // No clear color detected: sweep to find the path
  else {
    // Gentle sweep to look for path
    turnLeft();
    delay(100);
    turnRight();
    delay(200);
  }
}

// ================= COLOR SENSING =================
// Reads raw RED frequency (LOWER value = stronger red)
unsigned long readRedRaw() {
  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);
  return pulseIn(out, LOW);
}

unsigned long readGreenRaw() {
  digitalWrite(s2, HIGH);
  digitalWrite(s3, HIGH);
  return pulseIn(out, LOW);
}

unsigned long readBlueRaw() {
  digitalWrite(s2, LOW);
  digitalWrite(s3, HIGH);
  return pulseIn(out, LOW);
}

String detectColour() {
  unsigned long red   = readRedRaw();
  unsigned long green = readGreenRaw();
  unsigned long blue  = readBlueRaw();

  // Debug (USE THIS DURING CALIBRATION)
  Serial.print("R: "); Serial.print(red);
  Serial.print(" G: "); Serial.print(green);
  Serial.print(" B: "); Serial.println(blue);

  if (red < green && red < blue && red < 200) {
    return "red";
  } else if (green < red && green < blue && green < 200) {
    return "green";
  } else if (blue < red && blue < green && blue < 200) {
    return "blue";
  }
  // Check for black (all values high, indicating dark surface)
  else if (red > 150 && green > 150 && blue > 150) {
    return "black";
  }
  return "null";
}

// ================= THE "BIATHLON" SHOT =================
void shootBall() {
  // First, move forward until black is detected (on the ramp)
  while (detectColour() != "black") {
    moveForward(cruiseSpeed);
    delay(50);  // Small delay for sensor reading
  }
  
  stopMotors();
  delay(300);  // Stabilize on the ramp

  // Now proceed with the shot sequence
  // move forward exactly 26 cm (do calcs later)
  moveForward(cruiseSpeed);
  delay(800); // CHANGE TS
  stopMotors();

  // place ball down
  armDown();
  clawOpen();

  // go back just cuz yea
  moveBackward(cruiseSpeed);
  delay(800); // CHANGE TS (optional)

}

// ====================== BALL PICKUP/DROP =================
void pickUpBox() {
  // box is 90 deg to the right so we must rotate right

  turnRight(); // turn right for 500 ms (FIX TS)
  delay(500);

  clawOpen();   // open claw
  armDown();   // lower arm
  clawClose();   // close claw to grab box
  armUp();    // lift box with arm

  turnLeft(); // turn left to original orientation (FIX TS)
  delay(500);
  
  boxPickedUp = true;
}

void dropBox() {
  // box is 90 deg to the right so we must rotate right

  turnRight(); // turn right for 500 ms (FIX TS)
  delay(500);

  armDown();   // lower arm
  clawOpen();   // open claw to release box
  armUp();    // lift arm back up

  turnLeft(); // turn left to original orientation (FIX TS)
  delay(500);

  boxPickedUp = false;
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