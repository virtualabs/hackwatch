
# Hackwatch Installation

## Development framework

Install the Espressif SDK:
 * Get Espressif's ESP-IDF framework (version 4.4.2) and install it (following the instructions at https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/)
 * In the installation of ESP-IDF, make sure you specify `-b v4.4.2` when cloning
   ```
   mkdir -p ~/esp
   cd ~/esp
   git clone --recursive -b v4.4.2 https://github.com/espressif/esp-idf.git
   cd esp-idf/
   ./install.sh esp32
   . ./export.sh
   idf.py --version
   ```
 * Verify the output, it should be 4.4.2

## Hackwatch projet

Clone the hackwatch project from github:

    cd ~/esp/    
    git clone --recurse-submodules https://github.com/virtualabs/hackwatch
    cd hackwatch

Customize feature and select T-Watch 2020 version:
  * `idf.py menuconfig`
  * To decide what feature (Wifi, BLE and/or IR):
    * Choose `HackWatch menu`
  * To select your T-Watch version:
    * Choose `Component config`
    * Go at the very bottom and pick `T-Watch Lib`
    * Select `Target T-Watch version` to choose between `T-Watch 2020 v1`, `v2` or `v3`
    * (S)ave and quit

## Compile

To compile the project

    idf.py build

## Flash

Start the watch then connect it in USB to the computer. 

To flash the code execute this command:

    idf.py -p <your USB port> flash

## Debugging the firmware (monitor mode)

To monitor the execution:

    idf.py -p <your USB port> monitor


