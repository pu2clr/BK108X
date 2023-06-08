# This script uses the arduino-cli to compile the arduino sketches using command line (without Arduino IDE).
# It is very useful to check the library on multiple board types after bug fixes and improvements.
# It runs on a MacOS but you can port it to Linux or Windows easily.
# Some compilation results (bin and hex files) will be stores in your Download directory (/Users/<username>/Downloads/hex)
# ATTENTION: 1) Be careful with --output-dir arduino-cli option. Before compiling, this option removes all the contents of the last level folder. 
#               For example: if you use "--output-dir ~/Downloads", all the current content of the Downloads folder will be lost. 
#                         if you use "--output-dir ~/Downloads/hex", all current content of the hex folder will be lost and the Downloads 
#                         content will be preserved. 
#            2) I have enabled all the compile warnings (--warnings all) to check some questionable situations that can be avoided or modified to prevent future warnings.  
#            3) I have enabled  the "--verbose" parameter to show the deteiled logs of the compiling process.
# Please, see the file config_libraries_and_boards.sh
# Ricardo Lima Caratti Mar 2023

# compiles POC
echo "********************"
echo "Arduino ATmega328 based board"
echo "********************"
arduino-cli compile -b arduino:avr:nano ./BK1088_00_SERIAL_MONITOR/POC_01_CIRCUIT_TEST --output-dir ~/Downloads/hex/atmega/POC_01_CIRCUIT_TEST  --warnings all
arduino-cli compile -b arduino:avr:nano ./BK1088_00_SERIAL_MONITOR/POC_02_AM_FM --output-dir ~/Downloads/hex/atmega/POC_02_AM_FM  --warnings all
arduino-cli compile -b arduino:avr:nano ./BK1088_00_SERIAL_MONITOR/POC_03_SW --output-dir ~/Downloads/hex/atmega/POC_03_SW  --warnings all
arduino-cli compile -b arduino:avr:nano ./BK1088_02_TFT_DISPLAY/POC01_ST7735 --output-dir ~/Downloads/hex/atmega/POC01_ST7735  --warnings all


echo "********************"
echo "Arduino LGT8FX based board"
echo "********************"

arduino-cli compile -b lgt8fx:avr:328 ./BK1088_00_SERIAL_MONITOR/POC_01_CIRCUIT_TEST --output-dir ~/Downloads/hex/lgt8fx/POC_01_CIRCUIT_TEST  --warnings all
arduino-cli compile -b lgt8fx:avr:328 ./BK1088_00_SERIAL_MONITOR/POC_02_AM_FM --output-dir ~/Downloads/hex/lgt8fx/POC_02_AM_FM  
arduino-cli compile -b lgt8fx:avr:328 ./BK1088_00_SERIAL_MONITOR/POC03_SW --output-dir ~/Downloads/hex/lgt8fx/POC03_SW  

# compiles ESP32 LCD16x2_ALL_IN_ONE
echo "********************"
echo "ESP32"
echo "ESP32 LCD16x2"
echo "********************"
arduino-cli compile --fqbn esp32:esp32:esp32-poe-iso ./BK1088_03_LCD16X02_ESP32 --output-dir ~/Downloads/hex/ESP32/DEVM/BK1088_03_LCD16X02_ESP32  --warnings all
