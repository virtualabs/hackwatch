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

T-Watch 2020 v2 is a bit different:
 * remove Max98357a I2S amplifier
 * add DVR2605L haptic vibration controller 
 * add L76K GPS
 * add TF Card Holder

T-Watch 2020 v3 is very similar to v1 but add:
 * PDM microphone


How to build this project
-------------------------

See [INSTALL](INSTALL.md).


How to use it
-------------

Once flashed, the watch will restart and display the current time and date. Swipe left or right to browse the
main categories, swipe up on a category screen to access the different tools/screens available. Basically,
categories/tools navigation is done by swiping the screen :).

Pressing the button on the right side of the watch will bring you back to the current category and then to
the main screen. If this button is pressed when on the main screen (clock), the watch will go in deep sleep
mode and can be woken up by pressing the button again.

Provided tools
--------------

### WiFi

This smartwatch provides a real-time WiFi network scanner, a WiFi channel scanner, a rogue AP generator and
a WiFi deauth tool.

The WiFi Scanner shows a list of available wireless access points, along with their RSSI levels and MAC
addresses. Information about security and channel can be displayed when an access point is selected.

The WiFi channel scanner simply loops on every possible channel and shows the *activity*. It could be useful
to find less noisy channels when setting up a WiFi network.

The WiFi Rogue AP generator provides a way to spoof a specific WiFi access point (previously selected) in
order to force stations to connect to it instead of the legitimate access point. Security is set to open by
default, and no IP will be provided on connection (no DHCP server started, and no web server).

The WiFi Deauth tool allows you to automatically disconnect every connected station from a given WiFi 
network. This is quite similar to other projects like [ESP32 WiFi Penetration tool](https://github.com/risinek/esp32-wifi-penetration-tool) or [ESP32 Deauther](https://github.com/GANESH-ICMC/esp32-deauther).

### Bluetooth Low Energy

The smartwatch includes a BLE scanner that performs fingerprinting of detected devices and classifying
them in various categories (watch, health device, audio, etc.). It also provides a dedicated feature
to retrieve the Bluetooth Low Energy baseband vendor and software version (using BLE LL_VERSION_IND PDU).

### Infrared

A TV-B-Gone feature has been added to the watch. It can be started from the Infrared tool, and kept active
while the watch still remains on its main screen (stealth mode :D). 