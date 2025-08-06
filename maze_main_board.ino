#include <Servo.h> //Include the Servo library
#include <LedControl.h>
#include <SPI.h>
#include <MFRC522.h>


Servo myServo;  //Create a servo object to control the servo
Servo myServo1;
Servo myServo2;
Servo myServo3;
Servo myServo4;
Servo myServo5;
Servo myServo6;
Servo myServo7;
Servo myServo8;

const int ledPin = 39;
  
const int touch1 = 23; //Touch sensor 1 pin
const int touch2 = 25; //Touch sensor 2 pin

int sequence = 0;      
int winCount=0; //Variable for counting the number of wins both red and blue

//Define LEDs for both red and blue for the score system
//each round--> red and blue
const int blue1ledPin = 27;
const int red1ledPin = 29;
const int blue2ledPin = 31;
const int red2ledPin = 33;
const int blue3ledPin = 35;
const int red3ledPin = 37;

//Define LEDs for win and lose
const int redwinledPin = 18; //Blue loss pin
const int bluewinledPin = 17; //Red loss pin

//RFID sensor pins
#define RST_PIN 43
#define SS_PIN 41
MFRC522 mfrc522(SS_PIN, RST_PIN);

//Score variables
int redScore = 0;
int blueScore = 0;

//Flags for controlling LED and score updates
bool levelchange = false;
bool scorechange = false;

//RFID tag UIDs for red and blue objects
byte redTag[4] = {0x56, 0x79, 0xC8, 0x01}; // 56 79 C8 01 red
byte blueTag[4] = {0x02, 0x5D, 0x5C, 0x1B}; // 02 5D 5C 1B blue


void setup() {
  //Set servo motor pins as OUTPUT
  myServo.attach(9); // 1,3,4
  myServo1.attach(8); //2,5,6
  myServo2.attach(7); //7
  myServo3.attach(6); //10
  myServo4.attach(5); //9,12
  myServo5.attach(4); //11
  myServo6.attach(3); //8
  myServo7.attach(10); //11(middle)
  myServo8.attach(11); //12(middle)

  //Set LED pins as OUTPUT
  pinMode(ledPin, OUTPUT);//LED strip
  //Score system LEDs
  pinMode(blue1ledPin, OUTPUT);
  pinMode(red1ledPin, OUTPUT);
  pinMode(blue2ledPin, OUTPUT);
  pinMode(red2ledPin, OUTPUT);
  pinMode(blue3ledPin, OUTPUT);
  pinMode(red3ledPin, OUTPUT);
  //Win and lose LEDs
  pinMode(redwinledPin, OUTPUT);
  pinMode(bluewinledPin, OUTPUT);
  
  pinMode(touch1, INPUT); //Touch sensor 1 as INPUT
  pinMode(touch2, INPUT); //Touch sensor 2 as INPUT


  Serial.begin(9600);

  //Initialize RFID
  SPI.begin();
  mfrc522.PCD_Init();
  
  Serial.println("==================================");
  Serial.println("    RFID DETECTION SCORE CARD    ");
  Serial.println("==================================");
  Serial.println("Detecting RFID tags for RED/BLUE objects");
  Serial.println("==================================");
  Serial.println("Please scan your red and blue tags to set their UIDs:");
  Serial.println("==================================");
  Serial.println();
}


void loop() {
  //Check if both touch sensors are touched (start next sequence)
  if (digitalRead(touch1) == HIGH && digitalRead(touch2) == HIGH) {
    delay(500); //Debounce / prevent false trigger

    runSequence(sequence);//Activate servo sequence
    levelchange=true;
    scorechange=true;

    sequence = (sequence + 1) % 3; //Cycle through sequences (0 → 1 → 2)

    //Wait until fingers are removed from sensors
    while (digitalRead(touch1) == HIGH || digitalRead(touch2) == HIGH);
    delay(200); 
  }

  //RFID detection
  String detectedTag = readRFID();
  
  //If a RED tag is detected and score hasn't been updated yet
  if (detectedTag == "RED") {
    if(sequence==1 && scorechange){ 
      winCount=1;
      redScore++;
      scorechange=false;
    }
    else if(sequence==2 && scorechange){
      winCount=2;
      redScore++;
      scorechange=false;     
    }
    else if(sequence==0 && scorechange){
      winCount=3;
      redScore++;
      scorechange=false;    
    }
    //Light up only the LED for the current active level
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
      //display w or l
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
  //If a BLUE tag is detected and score hasn't been updated yet
  else if (detectedTag == "BLUE") {
    if(sequence==1 && scorechange){
      winCount=1;
      blueScore++;
      scorechange=false;   
    }
    else if(sequence==2 && scorechange){
        winCount=2;
        blueScore++;
      scorechange=false;     
    }
    else if(sequence==0 && scorechange){
        winCount=3;
        blueScore++;
      scorechange=false;
    }

    //Light up only the LED for the current active level
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
      //display w or l
      
      delay(5000); 
    }
     
    Serial.print("*** SCORE UPDATE *** Blue Team: ");
    Serial.println(blueScore);
    Serial.println("------------------------");
    
    delay(1500); //Prevent multiple detections
  }
  
  if (winCount==3 && redScore>=2){
    Serial.println("RED WINS");
    int x=1;
    while(x<=40){
      digitalWrite(redwinledPin, HIGH);
      delay(200);
      digitalWrite(redwinledPin, LOW);
      delay(200);
      x++;
    }

    resetScores();//Reset to 0   
  }
  else if (winCount==3 && blueScore>=2){
    Serial.println("BLUE WINS");
    
    int y=1;
    while(y<=40){
       digitalWrite(bluewinledPin, HIGH);
      delay(200);
      digitalWrite(bluewinledPin, LOW);
      delay(200);
      y++;
    }

    resetScores();//Reset to 0
  }
  delay(200); //Small delay for stability
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
  //Pattern for LED strip before a new level
  digitalWrite(ledPin, HIGH); //Turn LED ON
  delay(100);                 //Wait 1 second
  digitalWrite(ledPin, LOW);  //Turn LED OFF
  delay(100);                 //Wait 1 second
  digitalWrite(ledPin, HIGH); //Turn LED ON
  delay(100);                 //Wait 1 second
  digitalWrite(ledPin, LOW);  //Turn LED OFF
  delay(100);                 //Wait 1 second
  digitalWrite(ledPin, HIGH); //Turn LED ON

  //servo motors level changing for level 1
  myServo.write(0);
  myServo2.write(97);
  myServo3.write(0);
  myServo4.write(0);
  myServo5.write(0);
  myServo1.write(97);//5
  myServo7.write(97);
    
  delay(1500);//Avoid hitting nearby walls each other

  myServo6.write(97);//8
  myServo8.write(0);//12(middle)
}

void sequence2() {
  //Pattern for LED strip before a new level
  digitalWrite(ledPin, HIGH); //Turn LED ON
  delay(100);                 //Wait 1 second
  digitalWrite(ledPin, LOW);  //Turn LED OFF
  delay(100);                 //Wait 1 second
  digitalWrite(ledPin, HIGH); //Turn LED ON
  delay(100);                 //Wait 1 second
  digitalWrite(ledPin, LOW);  //Turn LED OFF
  delay(100);                 //Wait 1 second
  digitalWrite(ledPin, HIGH); //Turn LED ON

  //servo motors level changing for level 2
  myServo.write(0);
  myServo1.write(97);
  myServo2.write(0);
  myServo3.write(97);
  myServo4.write(90);
  myServo5.write(97);
  myServo6.write(97);
  myServo7.write(0);
  myServo8.write(0);
}

void sequence3() {
  //Pattern for LED strip before a new level
  digitalWrite(ledPin, HIGH); //Turn LED ON
  delay(100);                 //Wait 1 second
  digitalWrite(ledPin, LOW);  //Turn LED OFF
  delay(100);                 //Wait 1 second
  digitalWrite(ledPin, HIGH); //Turn LED ON
  delay(100);                 //Wait 1 second
  digitalWrite(ledPin, LOW);  //Turn LED OFF
  delay(100);                 //Wait 1 second
  digitalWrite(ledPin, HIGH); //Turn LED ON

  //servo motors level changing for level 3
  myServo2.write(97);
  myServo3.write(0);
  myServo4.write(0);
  myServo5.write(97);
  myServo6.write(0); //8
  myServo7.write(0);
  myServo8.write(97);//12
    
  delay(1500);//Avoid hitting nearby walls each other

  myServo1.write(3);//5
  myServo.write(97);//1,3,4
}

//RFID functions
String readRFID() {
  //Look for new tags
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return "NONE";
  }
  
  //Select one of the tags
  if (!mfrc522.PICC_ReadCardSerial()) {
    return "NONE";
  }
  
  //Get the UID
  byte readUID[4];
  for (int i = 0; i < 4; i++) {
    readUID[i] = mfrc522.uid.uidByte[i];
  }
  
  //Print UID for debugging
  Serial.print("Card detected! UID: ");
  for (int i = 0; i < 4; i++) {
    Serial.print(readUID[i], HEX);
    if (i < 3) Serial.print(":");
  }
  Serial.print(" -> ");
  
  //Compare with known tags
  String detectedTag = compareUID(readUID);
  Serial.println(detectedTag);
  
  //Halt communication with the card
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  
  return detectedTag;
}

String compareUID(byte* uid) {
  //Check if it's the red tag
  if (compareArrays(uid, redTag, 4)) {
    return "RED";
  }
  //Check if it's the blue tag
  if (compareArrays(uid, blueTag, 4)) {
    return "BLUE";
  }
  //If neither, it's unknown
  //You can use this section to set new tag UIDs
  Serial.print("Unknown tag detected. To set as RED tag, type 'R'. To set as BLUE tag, type 'B': ");
  return "UNKNOWN";
}


/
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