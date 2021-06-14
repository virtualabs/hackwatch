
# Hackwatch Installation

## Development framework

To install esp-idf framework, follow the procedure avaiable here:

    https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/#get-started-get-esp-idf

## Hackwatch projet

Clone the hackwatch project from github into the esp folder:

    cd ~/esp
    git clone --recurse-submodules https://github.com/virtualabs/hackwatch

## Compile

To compile the project:

    cd ~/esp/hackwatch
    idf.py build

## Flash

Start the watch then connect it in USB to the computer.

To flash the code execute this command:

    idf.py -p /dev/ttyUSB0 -b 460800 flash

## Monitor

To monitor the execution:

    idf.py -p /dev/ttyUSB0 monitor

