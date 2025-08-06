#include <WiFi.h>
#include <esp_now.h>
#include <Arduino.h>

// CAR CONFIGURATION - red
#define CAR_01
#ifdef CAR_01
  uint8_t JOYSTICK_MAC[6] = {0x00, 0x4B, 0x12, 0xEF, 0xA8, 0xF8};
#else
  uint8_t JOYSTICK_MAC[6] = {0xF4, 0x65, 0x0B, 0x55, 0xEF, 0xF4};
#endif


// Ultrasonic Sensors
#define TRIG1 33
#define ECHO1 32
#define TRIG2 14
#define ECHO2 27
#define TRIG3 26
#define ECHO3 25
#define TRIG4 13
#define ECHO4 12

// Motor Control Pins
#define IN1 18
#define IN2 19
#define IN3 21
#define IN4 22
#define ENA 23
#define ENB 5

// LED Indicator
#define LED_PIN 2



struct CarConfig {
  int forwardSpeed = 70;
  int backwardSpeed = 70;
  int leftSpeed = 70;
  int rightSpeed = 70;
  int frontThreshold = 3;
  int leftThreshold = 3;
  int rightThreshold = 3;
  int backThreshold = 3;
  int leftMotorBalance = 100;
  int rightMotorBalance = 100;
  bool obstacleAvoidanceEnabled = true;
};
CarConfig config;

typedef struct {
  int x;
  int y;
  bool button;
  bool connected;
} JoystickData;

JoystickData joystickData;
bool joystickConnected = false;
unsigned long lastJoystickTime = 0;

struct SensorData {
  float front;
  float left;
  float right;
  float back;
} sensors;

bool obstacleDetected = false;
String detectedObstacleLocation = "";

bool isMoving = false;
unsigned long movementStartTime = 0;
const unsigned long MOVEMENT_DURATION = 100;
const unsigned long STOP_DURATION = 300;
unsigned long lastStopTime = 0;
String currentDirection = "stop";

void setup() {
  Serial.begin(115200);

  pinMode(TRIG1, OUTPUT); pinMode(ECHO1, INPUT);
  pinMode(TRIG2, OUTPUT); pinMode(ECHO2, INPUT);
  pinMode(TRIG3, OUTPUT); pinMode(ECHO3, INPUT);
  pinMode(TRIG4, OUTPUT); pinMode(ECHO4, INPUT);

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT);

  pinMode(LED_PIN, OUTPUT);
  stopMotors();

  
  

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }
  esp_now_register_recv_cb(onDataReceived);
  connectToJoystick();
}

void loop() {
  readSensors();
  obstacleDetected = checkAnyObstacle();

  if (!joystickConnected && millis() - lastJoystickTime > 1000) {
    joystickConnected = false;
    stopMotors();
    Serial.println("Joystick disconnected");
    digitalWrite(LED_PIN, millis() % 1000 < 500);
  }

  if (joystickConnected) {
    processJoystickControl();
    digitalWrite(LED_PIN, HIGH);
  }

  delay(10);
}

void connectToJoystick() {
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, JOYSTICK_MAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);
}

void onDataReceived(const uint8_t* mac, const uint8_t* data, int len) {
  if (memcmp(mac, JOYSTICK_MAC, 6) != 0) return;
  if (len == sizeof(JoystickData)) {
    memcpy(&joystickData, data, sizeof(joystickData));
    joystickConnected = true;
    lastJoystickTime = millis();
  }
}

void readSensors() {
  sensors.front = readUltrasonic(TRIG1, ECHO1);
  delayMicroseconds(100);
  sensors.left = readUltrasonic(TRIG2, ECHO2);
  delayMicroseconds(100);
  sensors.right = readUltrasonic(TRIG3, ECHO3);
  delayMicroseconds(100);
  sensors.back = readUltrasonic(TRIG4, ECHO4);
}

float readUltrasonic(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(8);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH, 8000);
  return duration == 0 ? 999 : duration * 0.034 * 0.5;
}

bool checkObstacle(String direction) {
  if (direction == "front") return sensors.front < config.frontThreshold;
  if (direction == "back") return sensors.back < config.backThreshold;
  if (direction == "left") return sensors.left < config.leftThreshold;
  if (direction == "right") return sensors.right < config.rightThreshold;
  return false;
}

bool checkAnyObstacle() {
  if (!config.obstacleAvoidanceEnabled) return false;
  bool f = checkObstacle("front"), b = checkObstacle("back"),
       l = checkObstacle("left"), r = checkObstacle("right");
  detectedObstacleLocation = "";
  if (f) detectedObstacleLocation += "Front ";
  if (b) detectedObstacleLocation += "Back ";
  if (l) detectedObstacleLocation += "Left ";
  if (r) detectedObstacleLocation += "Right ";
  return f || b || l || r;
}

void applyMotorBalance(int baseSpeed, int &leftSpeed, int &rightSpeed) {
  leftSpeed = (baseSpeed * config.leftMotorBalance) / 100;
  rightSpeed = (baseSpeed * config.rightMotorBalance) / 100;
  leftSpeed = constrain(leftSpeed, 0, 255);
  rightSpeed = constrain(rightSpeed, 0, 255);
}

void moveForward() {
  if (config.obstacleAvoidanceEnabled && checkObstacle("front")) return stopMotors();
  int l, r; applyMotorBalance(config.forwardSpeed, l, r);
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  analogWrite(ENA, l); analogWrite(ENB, r);
}

void moveBackward() {
  if (config.obstacleAvoidanceEnabled && checkObstacle("back")) return stopMotors();
  int l, r; applyMotorBalance(config.backwardSpeed, l, r);
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  analogWrite(ENA, l); analogWrite(ENB, r);
}

void turnLeft() {
  if (config.obstacleAvoidanceEnabled && checkObstacle("left")) return stopMotors();
  int l, r; applyMotorBalance(config.leftSpeed, l, r);
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  analogWrite(ENA, l); analogWrite(ENB, r);
}

void turnRight() {
  if (config.obstacleAvoidanceEnabled && checkObstacle("right")) return stopMotors();
  int l, r; applyMotorBalance(config.rightSpeed, l, r);
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  analogWrite(ENA, l); analogWrite(ENB, r);
}

void stopMotors() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  analogWrite(ENA, 0); analogWrite(ENB, 0);
}

void processJoystickControl() {
  if (abs(joystickData.x) < 100 && abs(joystickData.y) < 100) {
    if (isMoving) stopMotors();
    isMoving = false; currentDirection = "stop";
    return;
  }

  String desiredDirection = "stop";
  if (joystickData.y > 100) desiredDirection = "forward";
  else if (joystickData.y < -100) desiredDirection = "backward";
  else if (joystickData.x > 100) desiredDirection = "right";
  else if (joystickData.x < -100) desiredDirection = "left";

  unsigned long now = millis();
  if (isMoving && now - movementStartTime >= MOVEMENT_DURATION) {
    stopMotors(); isMoving = false; lastStopTime = now; currentDirection = "stop";
  } else if (!isMoving && now - lastStopTime >= STOP_DURATION) {
    if (desiredDirection != "stop") {
      bool canMove = true;
      if (config.obstacleAvoidanceEnabled && checkObstacle(desiredDirection)) canMove = false;
      if (canMove) {
        if (desiredDirection == "forward") moveForward();
        else if (desiredDirection == "backward") moveBackward();
        else if (desiredDirection == "left") turnLeft();
        else if (desiredDirection == "right") turnRight();
        isMoving = true;
        movementStartTime = now;
        currentDirection = desiredDirection;
      }
    }
  }
}