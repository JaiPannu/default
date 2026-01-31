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
const int IR_LEFT  = A0;   // Left IR sensor
const int IR_RIGHT = A1;   // Right IR sensor

// ================= ULTRASONIC SENSOR =================
// Used for detecting obstacles in front of the robot
const int TRIG_PIN = 2;    // Sends ultrasonic pulse
const int ECHO_PIN = 3;    // Receives reflected pulse

// ================= SERVO MOTORS =================
#include <Servo.h>
Servo armServo;            // Controls arm up/down
Servo clawServo;           // Controls claw open/close
const int ARM_PIN  = 12;
const int CLAW_PIN = 13;

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

  // Attach servos to pins
  armServo.attach(ARM_PIN);
  clawServo.attach(CLAW_PIN);

  // Initial robot configuration
  //armUp();      // Lift arm so it doesn't drag
  //clawOpen();   // Open claw by default
}


// ================= MAIN LOOP =================
void loop() {

  // Measure distance to obstacle in front
  long distance = readUltrasonic();

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

  // Read IR sensors
  // LOW usually means "detecting surface"
  // HIGH usually means "no surface"

  int left = analogRead(IR_LEFT);
  int right = analogRead(IR_RIGHT);
  bool leftDetect = left < irThreshold;
  bool rightDetect = right < irThreshold;
  // Both sensors see the path → go straight
  if (left == LOW && right == LOW) {
    moveForward(100);  // Move forward for 100ms
  }

  // Right sensor lost path → drifted right → turn left
  else if (left == LOW && right == HIGH) {
    turnLeft(100);  // Turn left for 100ms
  }

  // Left sensor lost path → drifted left → turn right
  else if (left == HIGH && right == LOW) {
    turnRight(100);  // Turn right for 100ms
  }

  // Neither sensor sees path → stop (fail-safe)
  else {
    stopMotors(0);  // Stop immediately
  }

  /*
    TODO (IMPORTANT):
    This is where RED LINE FOLLOWING will go.

    Replace or augment this logic with:
    - color sensor reading
    - detect RED intensity
    - bias turning toward red line

    Example strategy:
    - If color sensor sees RED strongly → go forward
    - If RED fades → sweep left/right until RED is found again
  */
}


// ================= OBSTACLE AVOIDANCE =================
void avoidObstacle() {

  stopMotors(200); // stop for 200 ms to avoid collision

  turnLeft(400); // turn left

  moveForward(300); // move forward

  turnRight(400); // turn right

  moveForward(300); // bot should be clear, move forward

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
void armUp() {
  armServo.write(60);   // Adjust angle as needed
}

void armDown() {
  armServo.write(120);  // Adjust angle as needed
}

void clawOpen() {
  clawServo.write(90);
}

void clawClose() {
  clawServo.write(30);
}
