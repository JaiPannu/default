#include <iostream>
using namespace std;

const int IN1 = A1;   // Left motor direction pin 1
const int IN2 = A2;   // Left motor direction pin 2

const int IN3 = A3;  // Right motor direction pin 1
const int IN4 = A4;  // Right motor direction pin 2

// // ================= ULTRASONIC SENSOR =================
// // Used for detecting obstacles in front of the robot
const int TRIG_PIN = 7;  //Fixed  // Sends ultrasonic pulse //FIXED BY NITISH
const int ECHO_PIN = 6;  //Fixed  // Receives reflected pulse //FIXED BY NITISH

// Color sensor TCS3200 //FIXED BY NITISH
const int s0  = 8;
const int s1  = 9;
const int s2  = 10;
const int s3  = 11;
const int out = 12;

// ================= SETUP =================
void setup() {
  Serial.begin(9600);
  // Motor pins set as outputs
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

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
  digitalWrite(s1,LOW);
}


// ================= MAIN LOOP =================
void loop() {
  long distance_US = 0;
  String colour = " ";
  colour = detectColour();
  distance_US = readUltrasonic();
  if(distance_US < 35){
    turnRight(200);
    moveForward(300);
    turnLeft(150);
    while(detectColour() != "red"){
        moveForward(1);
    }
    moveForward(30);
    turnRight(50);
  }
  
  string colour = detectColour();

  if (colour == "red") {
    sweepCount = 0;
    sweepDirection = -1;
    moveForward(20);
    return;
  }

  if (colour == "blue" && !boxPickedUp) {
    pickUpBox();
    return;
  }

  if (colour == "blue" && boxPickedUp) {
    dropBox();
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


  /*
  Serial.print("Distance Ultrasonic: ");
  Serial.print(distance_US);
  Serial.print(", Colour: ");
  Serial.println(colour);
  delay(25);
  if (colour == "red")
  {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
  }
  else if (colour == "blue")
  {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
  }
  else if (colour == "green")
  {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
  }
  else if (colour == "black")
  {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
  }
  */

}


//================= ULTRASONIC DISTANCE FUNCTION =================
long readUltrasonic() {

  // Ensure clean pulse
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  // Send 10Âµs pulse
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Measure echo time (timeout = 25ms)
  long duration = pulseIn(ECHO_PIN, HIGH, 25000);

  return duration * 0.017;
}

String detectColour() {
  unsigned long red   = readRedRaw();
  unsigned long green = readGreenRaw();
  unsigned long blue  = readBlueRaw();

  // Debug (USE THIS DURING CALIBRATION)
  Serial.print("R: "); Serial.print(red);
  Serial.print(" G: "); Serial.print(green);
  Serial.print(" B: "); Serial.println(blue);
  //To detect red, 100 <= G <= 130
//To detect black, 150 <= G <= 173
//To detect green, 41 <= G <= 53
//To detect blue, 54 <= G <= 60
  if (blue >= 62 && blue <= 70) {
    return "red";
  } else if (blue >= 81 && blue <= 110) {
    return "black";
  } else if (blue >= 34 && blue <= 60) {
    return "green";
  } else if (blue >= 19 && blue <= 24) {
    return "blue";
  }
  return "null";
}
// ================= Color Sensing =================
// Reads raw RED frequency (LOWER value = stronger red)
unsigned long readRedRaw() {
  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);
  delay(10);
  return pulseIn(out, LOW);
}

unsigned long readGreenRaw() {
  digitalWrite(s2, HIGH);
  digitalWrite(s3, HIGH);
  delay(10);
  return pulseIn(out, LOW);
}

unsigned long readBlueRaw() {
  digitalWrite(s2, LOW);
  digitalWrite(s3, HIGH);
  delay(10);
  return pulseIn(out, LOW);
}

void moveForward(int duration) {

  // Both motors forward
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

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

  // Execute movement for specified duration
  delay(duration);
  stopMotors(0);
}

void turnLeft(int duration) {
  // Left motor backward
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  // Right motor forward
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  delay(duration);
  stopMotors(0);
}

void turnRight(int duration) {
  // Left motor forward
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  // Right motor backward
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

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