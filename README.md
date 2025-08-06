# Maze Master

## Description
Maze Master is a microcontroller-based maze game designed using Arduino Mega and ESP32 boards. The game features two cars controlled by joysticks, dynamic and static maze walls, and an RFID system to identify the winner after completing three rounds.

## Features
- Two-player maze game
- Dynamic and static maze walls
- Three game levels
- Two cars controlled via joysticks
- Center of the maze as the checkpoint for each round
- RFID-based winner identification after three rounds

## Hardware Components
- Arduino Mega (main game board controller)
- ESP32 (two for cars, two for joystick controllers)
- RFID module (MFRC522)
- Servo motors (for dynamic walls)
- Joystick modules
- Motor drivers
- DC motors
- Power supply and wiring components

## Software and Libraries
- Arduino IDE
- Servo.h
- LedControl.h
- SPI.h
- MFRC522.h
- WiFi.h
- esp_now.h
- Arduino.h

## File Structure
/MazeMaster
   ├── maze_main_board.ino   # Main controller logic
   ├── red_car.ino           # Red car control logic
   ├── blue_car.ino          # Blue car control logic
   ├── red_ctrl.ino          # Red joystick controller
   ├── blue_ctrl.ino         # Blue joystick controller
   └── README.md             # Project documentation