The HackWatch - a T-Watch 2020 firmware for WiFi and BLE hackers
================================================================

What about having a versatile WiFi/BLe hacking tool on your wrist ? Imagine this: you
walk into a room/building/premise/whatever and use your watch to scan/detect nearby WiFi networks,
BLE devices, and get information about them. Maybe you would also be able to try one or two
attacks that may be implemented into this firmware :).

What's this T-Watch 2020 ?
--------------------------

The T-Watch 2020 is an ESP32-based smartwatch for developper by Lilygo.

T-Watch 2020 v1 has:
 * a capacitive touch screen (240x240 pixels)
 * a 3-axis accelerometer
 * a speaker
 * a basic vibration motor 
 * a battery (of course)
 * an IR LED (well, could be useful)
 * an embedded real-time clock (obviously, it's a watch !)

T-Watch 2020 v2 is a bit diferent:
 * Max98357a I2S amplifier removed
 * DVR2605L haptip vibration contoler 
 * L76K GPS
 * TF Card Holder

T-Watch 2020 v3 is very similar to v1 but add:
 * PDM microphone

How to build this project
-------------------------

Install the Espressif SDK:
 * Get Espressif's ESP-IDF framework (version 4.4.2) and install it (following the instructions at https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/)
 
Clone the main repo:
* `git clone https://github.com/virtualabs/hackwatch`
* `cd hackwatch/`
* `git submodule update --init --recursive`

Customize feature and select T-Watch 2020 version:
* `idf.py menuconfig`
* To decide what feature (Wifi, BLE and/or IR):
  * Choose `HackWatch menu`
* To select your T-Watch version:
  * Choose `Component config`
  * Go at the very bottom and pick `T-Watch Lib`
  * Select `Target T-Watch version` to choose between `T-Watch 2020 v1`, `v2` or `v3`
 
Compile and Flash your watch:
 * type `idf.py build` to build this project
 * type `idf.py -p <your USB port> flash` to also flash your smartwatch
