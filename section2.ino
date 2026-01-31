// ================= MOTOR DRIVER PINS =================
// ENA / ENB control motor speed via PWM
// IN pins control motor direction
const int ENA = 5;   // Left motor speed
const int IN1 = 8;   // Left motor direction pin 1
const int IN2 = 9;   // Left motor direction pin 2

const int ENB = 6;   // Right motor speed
const int IN3 = 10;  // Right motor direction pin 1
const int IN4 = 11;  // Right motor direction pin 2

// ================= IR SENSORS =================
// Currently used as digital sensors (HIGH / LOW)
// You mentioned using IR for distance — if these are analog distance IR sensors,
// they should later be read using analogRead()
const int IR_RIGHT = A1;   // Right IR sensor

// ================= ULTRASONIC SENSOR =================
// Used for detecting obstacles in front of the robot
const int TRIG_PIN = 2;  //Fixed  // Sends ultrasonic pulse
const int ECHO_PIN = 3;  //Fixed  // Receives reflected pulse

// Color sensor TCS3200
const int s0  = 4;
const int s1  = 7;
const int s2  = A2;
const int s3  = A3;
const int out = A4;


// Sweep control variables
int sweepStep = 8;          // ms per micro turn (tune 5–15)
int sweepLimit = 6;         // how wide the sweep is
int sweepDirection = -1;    // -1 = left, +1 = right
int sweepCount = 0;


// ================= SERVO MOTORS =================
#include <Servo.h>
Servo armServo;            // Controls arm up/down
Servo clawServo;           // Controls claw open/close
const int ARM_PIN  = 10; // Fixed
const int CLAW_PIN = 11; // Fixed

// ================= MOVEMENT PARAMETERS =================
// Base forward speed (tune for stability vs speed)
int baseSpeed = 150;

// Speed difference when turning
int turnSpeed = 120;

// Distance threshold (cm) to trigger obstacle avoidance
int obstacleDistance = 20;

int irThreshold = 400; // determining when it is too left or too right

// ================= SETUP =================
void setup() {
  Serial.begin(9600);
  // Motor pins set as outputs
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // IR sensors set as inputs
  pinMode(IR_LEFT, INPUT);
  pinMode(IR_RIGHT, INPUT);

  // Ultrasonic pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // color sensor pins
  pinMode(s0,OUTPUT);    //pin modes
  pinMode(s1,OUTPUT);
  pinMode(s2,OUTPUT);
  pinMode(s3,OUTPUT);
  pinMode(out,INPUT);

  
  digitalWrite(s0,HIGH);
  digitalWrite(s1,HIGH);

  // Attach servos to pins
  armServo.attach(ARM_PIN);
  clawServo.attach(CLAW_PIN);

  // Initial robot configuration
  //moveArm(60);      // Lift arm so it doesn't drag
  //moveClaw(90);     // Open claw by default
}


// ================= MAIN LOOP =================
void loop() {

  // Measure distance to obstacle in front
  long distance = readUltrasonic();

  // priority 0: adjust speed based on distance
  baseSpeed = distance + 100;
  if (baseSpeed > 255) {
    baseSpeed = 255; // Cap at max PWM value
  }

  // PRIORITY 1: obstacle avoidance
  // If something is detected within threshold distance,
  // obstacle logic overrides path following
  if (distance > 0 && distance < obstacleDistance) {
    avoidObstacle();
  } 
  else {
    // PRIORITY 2: follow the path
    followPath();
  }

}


// ================= PATH FOLLOWING =================
void followPath() {
   if (redDetected()) {
    sweepCount = 0;
    sweepDirection = -1;
    moveForward(20);
    return;
  }

  if (sweepDirection == -1) {
    turnLeft(sweepStep);
  } else {
    turnRight(sweepStep);
  }

  sweepCount++;

  if (sweepCount >= sweepLimit) {
    sweepCount = 0;
    sweepDirection *= -1;  // flip direction
  }

}


// ================= OBSTACLE AVOIDANCE =================
void avoidObstacle() {

  stopMotors(200); // stop for 200 ms to avoid collision

  turnLeft(400); // turn left

  while(readIRRight()){
    moveForward(2);
  }
  moveForward(50);
  turnRight(400);

  while(readIRRight()){
    moveForward(2);
  }

  moveForward(100);
  turnRight(400);

  while(!(redDetected())){
    moveForward(2);
  }

  turnLeft(400);

}


// ================= ULTRASONIC DISTANCE FUNCTION =================
long readUltrasonic() {

  // Ensure clean pulse
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  // Send 10µs pulse
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Measure echo time (timeout = 25ms)
  long duration = pulseIn(ECHO_PIN, HIGH, 25000);

  // If no echo detected, return invalid distance
  if (duration == 0) return -1;

  // Convert time to distance in cm
  return duration * 0.034 / 2;
}


// ================= Color Sensing =================
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

bool redDetected() {
  unsigned long red   = readRedRaw();
  unsigned long green = readGreenRaw();
  unsigned long blue  = readBlueRaw();

  // Debug (USE THIS DURING CALIBRATION)
  Serial.print("R: "); Serial.print(red);
  Serial.print(" G: "); Serial.print(green);
  Serial.print(" B: "); Serial.println(blue);

  if (red < green && red < blue && red < 200) {
    return true;
  }
  return false;
}

// Read IR Right function
bool readIRRight() {
  int value = analogRead(IR_RIGHT);
  return value < irThreshold;  // closer = smaller value
}


// ================= MOTOR CONTROL FUNCTIONS =================
// All movement functions now accept a duration parameter (in milliseconds)
// This eliminates the need for delay() calls between movement commands

void moveForward(int duration) {

  // Both motors forward
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, baseSpeed);
  analogWrite(ENB, baseSpeed);

  // Execute movement for specified duration
  delay(duration);
  stopMotors(0);
}

void moveBackward(int duration) {

  // Both motors backward
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  analogWrite(ENA, baseSpeed);
  analogWrite(ENB, baseSpeed);

  // Execute movement for specified duration
  delay(duration);
  stopMotors(0);
}

void turnLeft(int duration) {

  // Slow left motor, fast right motor
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, turnSpeed);
  analogWrite(ENB, baseSpeed);

  // Execute turn for specified duration
  delay(duration);
  stopMotors(0);
}

void turnRight(int duration) {

  // Fast left motor, slow right motor
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, baseSpeed);
  analogWrite(ENB, turnSpeed);

  // Execute turn for specified duration
  delay(duration);
  stopMotors(0);
}

void stopMotors(int duration) {

  // Cut power to motors
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);

  // If duration > 0, maintain stopped state for specified time
  if (duration > 0) {
    delay(duration);
  }
} 


// ================= ARM & CLAW CONTROL =================
// Arm and claw control now use parameterized angles for flexibility

void moveArm(int angle) {
  armServo.write(angle);   // Set arm to desired angle
}

void moveClaw(int angle) {
  clawServo.write(angle);  // Set claw to desired angle
}
