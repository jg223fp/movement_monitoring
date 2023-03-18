# movement_monitoring
The idea for this project is to use an 8x8 IR thermal sensor for detection of people. The sensor has pixels just like a camera, but the 64 pixels are temperatures instead of colours. This hopefully gives the ability to calculate the direction of a humans movements and keep track of how many enters and how many leaves a room with just one sensor and without using a camera.

## Youtube link
- https://youtu.be/HnOBl7KIWlc


## Requirements
- Detect the presence of a heatsource(human).
- Detect the presence of a wireless device.
- Detect in what direction a heatsource(human) is moving.
- Send information to the cloud.
- Use a RTOS.

<br><br>


## Hardware components
- Raspberry pi pico w
- Adafruit AMG8833 thermal sensor
- Coloured LED
- Resitor 220 ohm
- Batterypack (Current battery is 1000 Mah)

<br><br>


## LoPy notes
1. While pins G23 and GND are shorted, reset the LoPy.

2. Upload the code from the Arduino IDE.

3. Remove the connection between G23 and GND and reset the LoPy again.

4. To init serial monitor just write something in the serial terminal

## wifi sniffer features:
- scans network 13 channels for 26 seconds.
- add found macs to ignore list
- then scans for macs and keeps track of count
- every mac gets a ttl of 60 seconds
- if ttl passes the mac is removed from the list
- every time the mac is found the ttl is reset to 60
- future improvement: add RSSI controll to remove packets with weak signals