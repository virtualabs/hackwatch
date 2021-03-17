The HackWatch - a T-Watch 2020 v1 firmware for WiFi and BLE hackers
===================================================================

What about having a versatile WiFi/BLe hacking tool on your wrist ? Imagine this: you
walk into a room/building/premise/whatever and use your watch to scan/detect nearby WiFi networks,
BLE devices, and get information about them. Maybe you would also be able to try one or two
attacks that may be implemented into this firmware :).

What's this T-Watch 2020 v1 ? Where to buy it ?
-----------------------------------------------

Well, it is basically an ESP32-based smartwatch with:

 * a capacitive touch screen (240x240 pixels)
 * a 3-axis accelerometer
 * a speaker
 * a battery (of course)
 * an IR LED (well, could be useful)
 * an embedded real-time clock (obviously, it's a watch !)

How to build this project
-------------------------

 * Get Espressif's ESP-IDF framework and install it (following the instructions at https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/)
 * type `idf.py build` or `idf.py -p <your USB port> flash` to build this project or/and flash your smartwatch


