# JKBMS-Fob

This project involves the development of a keyfob-like device powered by an Adafruit ESP32 Feather S3 TFT Reverse board, utilizing Bluetooth Low Energy (BLE) to communicate with JK BMS batteries. It allows users to read battery information and control charging, output, and balancing features directly from the keyfob.

## Features

- **Real-time Battery Monitoring:** Fetch and display real-time data from JK BMS batteries including voltage, current, temperature, and more.
- **Control Charging:** Enable or disable charging of the battery with a simple button press.
- **Control Discharge:** Enable or disable the output of the battery to connected devices.
- **Control Balancing:** Enable or disable the balancing feature of the battery.

## Hardware Requirements

- Adafruit ESP32 Feather S3 TFT Reverse board
- Lipo Battery size TBD
- JK BMS battery

## Software Requirements

- ESP-IDF v4.4.7
- Visual Studio Code

## Installation

1. Setup the ESP-IDF environment by following the instructions [here](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html).
2. Clone the repository.
3. Open the project in Visual Studio Code.
4. Build and flash the project to the ESP32 Feather S3 TFT Reverse board.

## Usage

1. Power on the Adafruit ESP32 Feather S3 TFT Reverse board.
2. Press the buttons on the keyfob to navigate through the menu and control the battery features.
3. The fob will automatically sleep after a period of inactivity to conserve battery life.


## Libraries

- 'LovyanGFX' - A library for the TFT display.
- 'esp-idf-lib' - A library for various ESP components.
## License
TODO