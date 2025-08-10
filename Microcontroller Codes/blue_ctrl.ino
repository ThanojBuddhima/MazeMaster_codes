#include <WiFi.h>
#include <esp_now.h>
#include <Arduino.h>

// JOYSTICK CONFIGURATION
// Define which joystick this is (uncomment one)
// #define JOYSTICK_01
#define JOYSTICK_02

#ifdef JOYSTICK_01
  // Joystick 01 pairs with red Car 
  uint8_t CAR_MAC[6] = {0x68, 0x25, 0xDD, 0x32, 0x80, 0xBC}; // Car 01 MAC
#else
  // Joystick 02 pairs with blue Car 
  uint8_t CAR_MAC[6] = {0x6C, 0xC8, 0x40, 0x8C, 0x41, 0x0C}; // Car 02 MAC
#endif

// Joystick Pins
#define VRx_PIN 34  
#define VRy_PIN 35  
#define SW_PIN 4    
#define LED_PIN 2   

// Calibration values
int centerX = 2048;  // Default center
int centerY = 2048;  // Default center
int deadzone = 100;  // Deadzone around center
bool calibrated = false;   

// Communication structure (must match car)
typedef struct {
  int x;
  int y;
  bool button;
  bool connected;
} JoystickData;

JoystickData joystickData;

bool isConnected = false;
unsigned long lastSendTime = 0;
unsigned long lastConnectionAttempt = 0;
const unsigned long sendInterval = 50; // Send data every 50ms
const unsigned long connectionRetryInterval = 5000; // Retry connection every 5 seconds

// Function declarations
void readJoystick();
void connectToCar();
void sendJoystickData();
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void calibrateJoystick();

void setup() {
  Serial.begin(115200);
  
 
  Serial.println("ESP32 Joystick Controller - Manual MAC");

  Serial.print("Configured Car MAC: ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X", CAR_MAC[i]);
    if (i < 5) Serial.print(":");
  }
  Serial.println();
  
  
  // Initialize pins
  pinMode(VRx_PIN, INPUT);
  pinMode(VRy_PIN, INPUT);
  pinMode(SW_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  
  // Calibrate joystick at startup
  calibrateJoystick();
  
  // Initialize WiFi in Station mode
  WiFi.mode(WIFI_STA);
  Serial.println("WiFi mode set to STA");
  
  // Print MAC address
  Serial.print("Joystick MAC Address: ");
  Serial.println(WiFi.macAddress());
  
  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  Serial.println("ESP-NOW initialized successfully");
  
  // Register callback for send status
  esp_now_register_send_cb(onDataSent);
  
  // Connect to car
  connectToCar();
  
  Serial.println("ESP32 Joystick Controller Ready");
  Serial.println("Attempting to connect to car...");
}

void loop() {
  readJoystick();
  
  // Try to reconnect if not connected
  if (!isConnected && millis() - lastConnectionAttempt > connectionRetryInterval) {
    Serial.println("Attempting to reconnect to car...");
    connectToCar();
    lastConnectionAttempt = millis();
  }
  
  // Send joystick data at regular intervals
  if (millis() - lastSendTime >= sendInterval) {
    sendJoystickData();
    lastSendTime = millis();
  }
  
  // LED status indication
  if (isConnected) {
    digitalWrite(LED_PIN, HIGH); // Solid on when connected
  } else {
    digitalWrite(LED_PIN, millis() % 500 < 250); // Blink when not connected
  }
  
  delay(10);
}

void readJoystick() {
  // Read analog values (0-4095)
  int rawX = analogRead(VRx_PIN);
  int rawY = analogRead(VRy_PIN);
  bool buttonPressed = !digitalRead(SW_PIN);
  
  // Convert to range using calibrated center values
  joystickData.x = rawX - centerX;
  joystickData.y = rawY - centerY;
  
  // Scale to -512 to +512 range
  joystickData.x = map(joystickData.x, -2048, 2048, -512, 512);
  joystickData.y = map(joystickData.y, -2048, 2048, -512, 512);
  
  // Apply deadzone around center
  if (abs(joystickData.x) < deadzone) joystickData.x = 0;
  if (abs(joystickData.y) < deadzone) joystickData.y = 0;
  
  // Constrain values
  joystickData.x = constrain(joystickData.x, -512, 512);
  joystickData.y = constrain(joystickData.y, -512, 512);
  
  joystickData.button = buttonPressed;
  joystickData.connected = true;
}

void connectToCar() {
  // Remove existing peer if any
  esp_now_del_peer(CAR_MAC);
  
  // Add car as peer
  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(esp_now_peer_info_t));
  memcpy(peerInfo.peer_addr, CAR_MAC, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  peerInfo.ifidx = WIFI_IF_STA;
  
  if (esp_now_add_peer(&peerInfo) == ESP_OK) {
    Serial.println("Car peer added successfully");
  } else {
    Serial.println("Failed to add car peer");
  }
}

void sendJoystickData() {
  esp_err_t result = esp_now_send(CAR_MAC, (uint8_t *) &joystickData, sizeof(joystickData));
  
  if (result != ESP_OK) {
    if (isConnected) {
      Serial.println("Failed to send data to car");
      isConnected = false;
    }
  }
}

void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // Check if the MAC address matches our configured car
  bool macMatch = true;
  for (int i = 0; i < 6; i++) {
    if (mac_addr[i] != CAR_MAC[i]) {
      macMatch = false;
      break;
    }
  }
  
  if (!macMatch) {
    return; // Ignore responses from other devices
  }
  
  if (status == ESP_NOW_SEND_SUCCESS) {
    if (!isConnected) {
      Serial.print("Successfully connected to car: ");
      for (int i = 0; i < 6; i++) {
        Serial.printf("%02X", CAR_MAC[i]);
        if (i < 5) Serial.print(":");
      }
      Serial.println();
      isConnected = true;
    }
  } else {
    if (isConnected) {
      Serial.println("Lost connection to car");
      isConnected = false;
    }
  }
}

void calibrateJoystick() {
  
  Serial.println("    JOYSTICK CALIBRATION");
  
  Serial.println("Please keep joystick at CENTER position");
  Serial.println("Calibration will start in 3 seconds...");
  
  // LED blink pattern during calibration
  for (int i = 3; i > 0; i--) {
    Serial.print("Calibrating in ");
    Serial.print(i);
    Serial.println(" seconds...");
    digitalWrite(LED_PIN, HIGH);
    delay(300);
    digitalWrite(LED_PIN, LOW);
    delay(700);
  }
  
  Serial.println("Calibrating center position...");
  digitalWrite(LED_PIN, HIGH);
  
  // Take multiple readings and average them
  long sumX = 0, sumY = 0;
  int samples = 50;
  
  for (int i = 0; i < samples; i++) {
    sumX += analogRead(VRx_PIN);
    sumY += analogRead(VRy_PIN);
    delay(20);
  }
  
  centerX = sumX / samples;
  centerY = sumY / samples;
  
  digitalWrite(LED_PIN, LOW);
  
  Serial.println("Calibration Complete!");
  Serial.print("Center X: ");
  Serial.println(centerX);
  Serial.print("Center Y: ");
  Serial.println(centerY);

  calibrated = true;
  
  // Flash LED to indicate successful calibration
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(100);
  }
}
