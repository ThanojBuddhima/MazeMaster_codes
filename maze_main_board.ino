#include <Servo.h> // Include the Servo library
#include <LedControl.h>
#include <SPI.h>
#include <MFRC522.h>

Servo myServo;  // Create a servo object to control the servo
Servo myServo1;
Servo myServo2;
Servo myServo3;
Servo myServo4;
Servo myServo5;
Servo myServo6;
Servo myServo7;
Servo myServo8;

const int ledPin = 39;

const int touch1 = 23; // Touch sensor 1 pin
const int touch2 = 25; // Touch sensor 2 pin
int sequence = 0;      

int winCount=0;

bool levelmarks=true;

const int blue1ledPin = 27;
const int red1ledPin = 29;
const int blue2ledPin = 31;
const int red2ledPin = 33;
const int blue3ledPin = 35;
const int red3ledPin = 37;

const int redwinledPin = 18;//blue loss pin
const int bluewinledPin = 17;// red loss pin

// RFID sensor pins
#define RST_PIN 43
#define SS_PIN 41
MFRC522 mfrc522(SS_PIN, RST_PIN);

// Score variables
int redScore = 0;
int blueScore = 0;

bool levelchange = false;
bool scorechange = false;

// RFID tag UIDs for red and blue objects
byte redTag[4] = {0x56, 0x79, 0xC8, 0x01}; // 56 79 C8 01 red
byte blueTag[4] = {0x02, 0x5D, 0x5C, 0x1B}; // 02 5D 5C 1B blue

void setup() {
  myServo.attach(9);
  myServo1.attach(8);
  myServo2.attach(7);
  myServo3.attach(6);
  myServo4.attach(5);
  myServo5.attach(4);
  myServo6.attach(3);
  myServo7.attach(10);
  myServo8.attach(11);

  pinMode(ledPin, OUTPUT);
  pinMode(blue1ledPin, OUTPUT);
  pinMode(red1ledPin, OUTPUT);
  pinMode(blue2ledPin, OUTPUT);
  pinMode(red2ledPin, OUTPUT);
  pinMode(blue3ledPin, OUTPUT);
  pinMode(red3ledPin, OUTPUT);
  pinMode(redwinledPin, OUTPUT);
  pinMode(bluewinledPin, OUTPUT);
  pinMode(touch1, INPUT);
  pinMode(touch2, INPUT);

  Serial.begin(9600);
  
  // Initialize RFID
  SPI.begin();
  mfrc522.PCD_Init();

  Serial.println("==================================");
  Serial.println("    RFID DETECTION SCORE CARD    ");
  Serial.println("==================================");
  Serial.println("Red Team: Matrix 1, Blue Team: Matrix 2");
  Serial.println("Detecting RFID tags for RED/BLUE objects");
  Serial.println("==================================");
  Serial.println("Please scan your red and blue tags to set their UIDs:");
  Serial.println("==================================");
  Serial.println();
}

void loop() {
  
  if (digitalRead(touch1) == HIGH && digitalRead(touch2) == HIGH) {
    delay(500); // manage electripulse
    if(levelmarks){
      runSequence(sequence);
      levelchange=true;
      scorechange=true;

      sequence = (sequence + 1) % 3;
      levelmarks = false;
    }
    

    while (digitalRead(touch1) == HIGH || digitalRead(touch2) == HIGH);
    delay(200); 
  }

  // RFID detection
  String detectedTag = readRFID();
  
  // Only add scores for RED and BLUE tag detection
  if (detectedTag == "RED") {
    if(sequence==1 && scorechange){ 
      winCount=1;
      redScore++;
      scorechange=false;
      levelmarks = true;
    }
    else if(sequence==2 && scorechange){
      winCount=2;
      redScore++;
      scorechange=false;
      levelmarks = true;
    }
    else if(sequence==0 && scorechange){
      winCount=3;
      redScore++;
      scorechange=false;
      levelmarks = true;
    }
    if(winCount==1 && levelchange){
        digitalWrite(red1ledPin, HIGH);
        levelchange=false;
    }
    else if(winCount==2 &&  levelchange){
      digitalWrite(red2ledPin, HIGH);
      levelchange=false;
    }
    else if (winCount == 3 && levelchange){
      digitalWrite(red3ledPin, HIGH);
      levelchange=false;
      
      Serial.print("*** SCORE UPDATE *** RED Team: ");
      Serial.println(redScore);
      Serial.println("------------------------");
      
      delay(5000);
    }  
    
    Serial.print("*** SCORE UPDATE *** Red Team: ");
    Serial.println(redScore);
    Serial.println("------------------------");
    
    delay(1500); // Prevent multiple detections
  }
  else if (detectedTag == "BLUE") {
    if(sequence==1 && scorechange){
      winCount=1;
      blueScore++;
      scorechange=false;
      levelmarks = true;

    }
    else if(sequence==2 && scorechange){
        winCount=2;
        blueScore++;
        scorechange=false;
        levelmarks = true;
    }
    else if(sequence==0 && scorechange){
        winCount=3;
        blueScore++;
        scorechange=false;
        levelmarks = true;
    }
    if (winCount == 1 && levelchange){
      digitalWrite(blue1ledPin, HIGH);
      levelchange=false;
    }
    else if(winCount == 2 && levelchange){
      digitalWrite(blue2ledPin, HIGH);
      levelchange=false;
    }
    else if (winCount == 3 && levelchange) {
      digitalWrite(blue3ledPin, HIGH);
      levelchange=false;

      Serial.print("*** SCORE UPDATE *** Blue Team: ");
      Serial.println(blueScore);
      Serial.println("------------------------");
      
      delay(5000);
    }
    
    Serial.print("*** SCORE UPDATE *** Blue Team: ");
    Serial.println(blueScore);
    Serial.println("------------------------");
    
    delay(1500); // Prevent multiple detections
  }
  
  if (winCount==3 && redScore>=2){
    Serial.println("RED WINS");
    Serial.println("AUDIO:WIN_RED"); // Audio command for Python
    
    int x=1;
    while(x<=40){
      digitalWrite(redwinledPin, HIGH);
      delay(200);
      digitalWrite(redwinledPin, LOW);
      delay(200);
      x++;
    }
    
    resetScores();// Reset to 0 
  }
  else if (winCount==3 && blueScore>=2){
    Serial.println("BLUE WINS");
    Serial.println("AUDIO:WIN_BLUE"); // Audio command for Python
    
    int y=1;
    while(y<=40){
       digitalWrite(bluewinledPin, HIGH);
      delay(200);
      digitalWrite(bluewinledPin, LOW);
      delay(200);
      y++;
    }
    
    resetScores();// Reset to 0
  }

  delay(200); // Small delay for stability
}

void runSequence(int step) {
  switch (step) {
    case 0:
      sequence1();
      break;
    case 1:
      sequence2();
      break;
    case 2:
      sequence3();
      break;
  }
}

void sequence1() {
  digitalWrite(ledPin, HIGH);  // Turn LED ON
  delay(100);                 // Wait 1 second
  digitalWrite(ledPin, LOW);   // Turn LED OFF
  delay(100); 
  digitalWrite(ledPin, HIGH);  // Turn LED ON
  delay(100);                 // Wait 1 second
  digitalWrite(ledPin, LOW);   // Turn LED OFF
  delay(100); 
  digitalWrite(ledPin, HIGH);  // Turn LED ON

  //servo motors level changing
  myServo.write(0); // Set the servo position
  myServo2.write(97);
  myServo3.write(0);
  myServo4.write(0);
  myServo5.write(0);
  myServo1.write(97);//5
  myServo7.write(97);
    
  delay(1500);
  myServo6.write(97);//8
  myServo8.write(0);//12(middle)

  // Send audio command to Python
  Serial.println("AUDIO:TRACK_1");
}

void sequence2() {
  digitalWrite(ledPin, HIGH);  // Turn LED ON
  delay(100);                 // Wait 1 second
  digitalWrite(ledPin, LOW);   // Turn LED OFF
  delay(100); 
  digitalWrite(ledPin, HIGH);  // Turn LED ON
  delay(100);                 // Wait 1 second
  digitalWrite(ledPin, LOW);   // Turn LED OFF
  delay(100); 
  digitalWrite(ledPin, HIGH);  // Turn LED ON
  
  //servo motors level changing
  myServo.write(0); // Set the servo position
  myServo1.write(97);
  myServo2.write(0);
  myServo3.write(97);
  myServo4.write(90);
  myServo5.write(97);
  myServo6.write(97);
  myServo7.write(0);
  myServo8.write(0);

  // Send audio command to Python
  Serial.println("AUDIO:TRACK_2");
}

void sequence3() {
  digitalWrite(ledPin, HIGH);  // Turn LED ON
  delay(100);                 // Wait 1 second
  digitalWrite(ledPin, LOW);   // Turn LED OFF
  delay(100); 
  digitalWrite(ledPin, HIGH);  // Turn LED ON
  delay(100);                 // Wait 1 second
  digitalWrite(ledPin, LOW);   // Turn LED OFF
  delay(100); 
  digitalWrite(ledPin, HIGH);  // Turn LED ON

  //servo motors level changing
  myServo2.write(97);
  myServo3.write(0);
  myServo4.write(0);
  myServo5.write(97);
  myServo6.write(0); //8
  myServo7.write(0);
  myServo8.write(97);//12
    
  delay(1500);
  myServo1.write(3);//5
  myServo.write(97);//1,3,4

  // Send audio command to Python
  Serial.println("AUDIO:TRACK_3");
}

// RFID functions
String readRFID() {
  // Look for new cards
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return "NONE";
  }
  
  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) {
    return "NONE";
  }
  
  // Get the UID
  byte readUID[4];
  for (int i = 0; i < 4; i++) {
    readUID[i] = mfrc522.uid.uidByte[i];
  }
  
  // Print UID for debugging
  Serial.print("Card detected! UID: ");
  for (int i = 0; i < 4; i++) {
    Serial.print(readUID[i], HEX);
    if (i < 3) Serial.print(":");
  }
  Serial.print(" -> ");
  
  // Compare with known tags
  String detectedTag = compareUID(readUID);
  Serial.println(detectedTag);
  
  // Halt communication with the card
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  
  return detectedTag;
}

String compareUID(byte* uid) {
  // Check if it's the red tag
  if (compareArrays(uid, redTag, 4)) {
    return "RED";
  }
  
  // Check if it's the blue tag
  if (compareArrays(uid, blueTag, 4)) {
    return "BLUE";
  }
  
  // If neither, it's unknown
  Serial.print("Unknown tag detected. To set as RED tag, type 'R'. To set as BLUE tag, type 'B': ");
  return "UNKNOWN";
}

bool compareArrays(byte* arr1, byte* arr2, int length) {
  for (int i = 0; i < length; i++) {
    if (arr1[i] != arr2[i]) {
      return false;
    }
  }
  return true;
}

// Function to manually reset scores (optional)
void resetScores() {
  redScore = 0;
  blueScore = 0;
  winCount=0;
  digitalWrite(red1ledPin, LOW);
  digitalWrite(red2ledPin, LOW);
  digitalWrite(red3ledPin, LOW);
  digitalWrite(blue1ledPin, LOW);
  digitalWrite(blue2ledPin, LOW);
  digitalWrite(blue3ledPin, LOW);
  Serial.println("Scores reset!");
}
