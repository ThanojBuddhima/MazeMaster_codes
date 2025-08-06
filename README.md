# Maze Master

## Description
**Maze Master** is a microcontroller-based two-player maze game designed using Arduino Mega and ESP32 boards. Players control cars through a maze filled with both static and dynamic walls. The goal is to reach the center checkpoint in each level. The first car to reach the center three times wins. The game includes RFID-based winner identification, wall-avoidance using ultrasonic sensors, and a touch sensor-based start system.

## Features
- Two-player interactive maze game  
- Three progressively challenging game levels  
- Dynamic and static maze wall system controlled by servo motors  
- Two cars wirelessly controlled via joystick controllers (ESP32)  
- Center maze checkpoint for round completion  
- Ultrasonic sensors to detect walls and prevent car collisions  
- RFID system for identifying which player reaches the center  
- LED indicators for level transitions, scoring, and game feedback  
- Touch sensor to start/reset the game  

## Hardware Components
- **Arduino Mega** – Main controller for dynamic walls and system coordination  
- **ESP32 (x4)** –  
  - 2 for cars (Red & Blue)  
  - 2 for joystick controllers  
- **RFID Module (MFRC522)** – To identify which car reaches the center checkpoint  
- **Servo Motors** – To rotate dynamic maze walls between levels  
- **DC Motors** – For car movement  
- **Motor Drivers** – To control DC motors  
- **Ultrasonic Sensors** – To detect walls and prevent cars from crashing  
- **Joystick Modules** – For player control of the cars  
- **Touch Sensors** – To start or reset the game  
- **LED System** – Visual indication of game events (start, level up, round complete, win/loss)  
- Power supply and wiring components  

## Software and Libraries
- **Arduino IDE** – For programming all boards  
- **Libraries Used**:  
  - `Servo.h` – Control servo motors for wall movement  
  - `LedControl.h` – Manage LED indicators  
  - `SPI.h` – Required for RFID and other SPI devices  
  - `MFRC522.h` – Interface with the RFID module  
  - `WiFi.h`, `esp_now.h` – For ESP32 wireless communication  
  - `Arduino.h` – Standard core Arduino functions  

## File Structure

```
/MazeMaster
├── maze_main_board.ino   # Controls maze walls, RFID detection, game logic
├── red_car.ino           # Controls Red car movement and sensor input
├── blue_car.ino          # Controls Blue car movement and sensor input
├── red_ctrl.ino          # Red joystick controller logic
├── blue_ctrl.ino         # Blue joystick controller logic
└── README.md             # Project documentation
```
