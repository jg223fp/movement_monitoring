# movement_monitoring
The idea for this project is to use an 8x8 IR thermal sensor for detection of people. The sensor has pixels just like a camera, but the 64 pixels are temperatures instead of colours. This hopefully gives the ability to calculate the direction of a humans movements and can there by keep track of how many enters and how many leaves a room with just one sensor and without using a camera.

## Requirements
- Detect the presence of a source of heat(human).
- Detect the presence of a wireless device.
- Detect in what direction a source of heat(human) is moving.
- Send information to the cloud.
- Use a RTOS.

<br><br>

## Software components
- Cmake 3.25.2
- arm-gnu-toolchain-12.2
- winlibs personal build version gcc-12.2.0-llvm-15.0.7-mingw-w64ucrt-10.0.0-r4
- FreeRTOS-Kernel
- pico-sdk


<br><br>

## Hardware components
- Raspberry pi pico w
- Adafruit AMG8833 thermal sensor
- Coloured LED
- Resitor 220 ohm
- Batterypack

<br><br>

## Scenario
Technique used for monitoring visitor flows is wanted for various areas. This projects uses a classroom as a scenario.