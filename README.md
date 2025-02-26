# motor-on-roller-blind-ws
WebSocket based version of [motor-on-roller-blind](https://github.com/nidayand/motor-on-roller-blind). I.e. there is no need of an MQTT server but MQTT is supported as well - you can control it with WebSockets and/or with MQTT messages.

3d parts for printing are available on Thingiverse.com: ["motor on a roller blind"](https://www.thingiverse.com/thing:2392856)

 1. A tiny webserver is setup on the esp8266 that will serve one page to the client
 2. Upon powering on the first time WIFI credentials, a hostname and - optional - MQTT server details is to be configured. You can specify if you want **clockwise (CW) rotation** to close the blind and you can also specify **MQTT authentication** if required. Connect your computer to a new WIFI hotspot named **BlindsConnectAP**. Password = **nidayand**
 3. Login to WiFiManager at 192.168.4.1
 4. Enter setup information (wifi credentials, device name, MQTT server info).  Device name will be automatically prefixed with 'blind-'.
 3. Connect to your normal WIFI with your client and go to the IP address of the device - or if you have an mDNS supported device (e.g. iOS, OSX or have Bonjour installed) you can go to http://{hostname}.local. If you don't know the IP-address of the device check your router for the leases (or check the serial console in the Arduino IDE or check the `/raw/esp8266/register` MQTT message if you are using an MQTT server)
 4. As the webpage is loaded it will connect through a websocket directly to the device to progress updates and to control the device. If any other client connects the updates will be in sync.
 5. Go to the Settings page to calibrate the motor with the start and end positions of the roller blind. Follow the instructions on the page.

## Flashing
This project is set-up in plattformIO (VSCode) but there is no need to pull the source code and build the project yourself. A binary is available in the *bin* folder. To flash it use esptool and run the following command:
> esptool.py --port <*replace with your com port*> write_flash 0x00000 bin/firmware.bin

### OTA updates
As well as providing flashing through wifi (ArduinoOTA) a binary can also be uploaded to the board via *http://board_name:82/*. 

## Reset
To reset the board settings you can ground D5 for a few seconds while powering it up.  The board will reboot itself when done and publish its SSID.

## MQTT
- When it connects to WIFI and MQTT it will send a "register" message to topic `/raw/esp8266/register` with a payload containing chip-id and IP-address
- A message to `/raw/esp8266/[chip-id]/in` will steer the blind according to the "payload actions" below
- Updates from the device will be sent to topic `/raw/esp8266/[chip-id]/out`

### If you don't want to use MQTT
Simply do not enter any string in the MQTT server form field upon WIFI configuration of the device (step 3 above)

### Payload options
Publish to `blind/[device-name]/in` with one of the following payloads:
- ***(start)*** - (calibrate) Sets the current position as top position
- ***(max)*** - (calibrate) Sets the current position as max position. Set `start` before you define `max` as `max` is a relative position to `start`
- ***(0)*** - (manual mode) Will stop the curtain
- ***(-1)*** - (manual mode) Will open the curtain. Requires `(0)` to stop the motor
- ***(1)***- (manual mode) Will close the curtain. Requires `(0)` to stop the motor
- ***0-100*** - (auto mode) A number between 0-100 to set % of covering. Requires calibration before use. 0% is fully open (this is backwards from Home Assistant).
- ***REBOOT*** - Reboot.
- ***RESET-CONFIG*** - Reset configuration, will reboot into WiFiManager AP mode.
- ***downspeed=x*** 5 by default. This will be stored in flash
- ***upspeed=x*** 5 by default. This will be stored in flash
- ***invert=x*** 0 or 1 for regular or inverted mode. This will be stored in flash

## Required libraries (3rd party)
*All* required libraries are included as git submodules. Clone this library with submodules.
- [Stepper_28BYJ_48](https://github.com/thomasfredericks/Stepper_28BYJ_48/)
- [PubSubClient](https://github.com/knolleary/pubsubclient/)
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
- [WIFIManager](https://github.com/tzapu/WiFiManager)
- [WebSockets](https://github.com/Links2004/arduinoWebSockets)
- [ESP8266_mdns](https://github.com/mrdunk/esp8266_mdns)
 

## Screenshots

### Control
![Control](https://user-images.githubusercontent.com/2181965/31178217-a5351678-a918-11e7-9611-3e8256c873a4.png)

### Calibrate
![Settings](https://user-images.githubusercontent.com/2181965/31178216-a4f7194a-a918-11e7-85dd-8e189cfc031c.png)

### Communication settings
 ![WIFI Manager](https://user-images.githubusercontent.com/2181965/37288794-75244c84-2608-11e8-8c27-a17e1e854761.jpg)
