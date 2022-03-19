# Car Data Collector
Code used to collect data from a car or any other CAN bus. This allows you to log CAN Bus data.

# Features
This is meant to be an easy-to-use system with minimum frills. It has two modes: Live data and logging. When you turn it on, it automatically begins logging all messages.

## Real-Time Compression
There is a lot of data that goes through the CAN Bus. It can be more than the rate at which the ESP32 is able to write to the SD card. Therefore, the ESP does real-time compression before writing. It decompresses the data automatically when you download it on the website.

## Livestream
It is able to stream the data coming through the CAN bus live on the website

## Timestamps
If the GPS ever connects to the satellites, it will be able to get the accurate time for all messages logged, including CAN

## Captive Portal
While not set as default, you can configure the device to send you to a captive portal with the same data downloads and livestream. This means you won't have to remember the url.

# Hardware Required
- Any ESP32 (Must be an ESP32 though, it requires too much power for any other application)
- SD card and reader
- SN65HVD230 CAN Bus Module
- GT-U7 GPS Module

# How to Set Up
## Wiring
- CAN Bus module: 
 - VCC -> 3v3
 - GND -> GND
 - CTX -> D5
 - CRX -> D4

- GPS Module
 - VCC -> 3v3
 - GND -> GND
 - TX -> 
 - RX ->

## Software
- Download the Arduino IDE, along with the necessary libraries
- Connect the ESP32 to your computer and upload the sketch.

## Downloading Data
- Connect to the access point made by the ESP32 (default "CAN LOGGER")
- Connect to esp32.local or 192.168.4.1
- Select what you want to view/download
