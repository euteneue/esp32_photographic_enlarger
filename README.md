# ESP32 Photographic Enlarger Controller

This project implements a photographic enlarger controller using an ESP32 microcontroller, utilizing the f-stop method for calculating enlarging time.

## Features
- F-stop based exposure time calculation
- TM1638 7-segment display, LEDs, and buttons
- Two rotary encoders for mode switching and value adjustment
- Relay control for enlarger
- Modular architecture with FreeRTOS tasks

## Hardware Requirements
- ESP32 development board
- TM1638 module
- Two rotary encoders with push buttons
- Relay module
- Connecting wires

## Software Architecture
- **HAL (Hardware Abstraction Layer)**: Relay, TM1638, Rotary Encoders
- **Application Logic**: Exposure Calculator, Finite State Machine
- **RTOS Tasks**: Input Handler, Display Update, Exposure Timer, State Manager

## Building and Flashing
1. Install PlatformIO
2. Open the project in VS Code with PlatformIO extension
3. Build and upload to ESP32

## Usage
- Use rotary encoders to adjust values in different modes
- Press buttons to switch modes
- Modes: Set Base Time, Set F-Stop, Calculate Time, Expose