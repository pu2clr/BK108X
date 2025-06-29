@echo off
REM This script uses the arduino-cli to compile the arduino sketches using command line (without Arduino IDE).
REM It is very useful to check the library on multiple board types after bug fixes and improvements.
REM It runs on Windows but you can port it to Linux or MacOS easily.
REM Some compilation results (bin and hex files) will be stored in your Downloads directory (%USERPROFILE%\Downloads\hex)
REM ATTENTION: 1) Be careful with --output-dir arduino-cli option. Before compiling, this option removes all the contents of the last level folder. 
REM               For example: if you use "--output-dir %USERPROFILE%\Downloads", all the current content of the Downloads folder will be lost. 
REM                         if you use "--output-dir %USERPROFILE%\Downloads\hex", all current content of the hex folder will be lost and the Downloads 
REM                         content will be preserved. 
REM            2) I have enabled all the compile warnings (--warnings all) to check some questionable situations that can be avoided or modified to prevent future warnings.  
REM            3) I have enabled  the "--verbose" parameter to show the detailed logs of the compiling process.
REM Please, see the file config_libraries_and_boards.bat
REM Ricardo Lima Caratti Mar 2023

REM compiles POC
echo ********************
echo Arduino ATmega328 based board
echo ********************
arduino-cli compile -b arduino:avr:nano .\BK1088_00_SERIAL_MONITOR\POC_01_CIRCUIT_TEST --output-dir %USERPROFILE%\Downloads\hex\atmega\POC_01_CIRCUIT_TEST  --warnings all
arduino-cli compile -b arduino:avr:nano .\BK1088_00_SERIAL_MONITOR\POC_02_AM_FM --output-dir %USERPROFILE%\Downloads\hex\atmega\POC_02_AM_FM  --warnings all
arduino-cli compile -b arduino:avr:nano .\BK1088_00_SERIAL_MONITOR\POC_03_SW --output-dir %USERPROFILE%\Downloads\hex\atmega\POC_03_SW  --warnings all
arduino-cli compile -b arduino:avr:nano .\BK1088_02_TFT_DISPLAY\POC_01_ST7735 --output-dir %USERPROFILE%\Downloads\hex\atmega\POC_01_ST7735  --warnings all
arduino-cli compile -b arduino:avr:nano .\BK1088_04_NOKIA5110_RDS_NANO --output-dir %USERPROFILE%\Downloads\hex\atmega\BK1088_04_NOKIA5110_RDS_NANO  --warnings all

echo.
echo ********************
echo Arduino LGT8FX based board
echo ********************

arduino-cli compile -b lgt8fx:avr:328 .\BK1088_00_SERIAL_MONITOR\POC_01_CIRCUIT_TEST --output-dir %USERPROFILE%\Downloads\hex\lgt8fx\POC_01_CIRCUIT_TEST  --warnings all
arduino-cli compile -b lgt8fx:avr:328 .\BK1088_00_SERIAL_MONITOR\POC_02_AM_FM --output-dir %USERPROFILE%\Downloads\hex\lgt8fx\POC_02_AM_FM  
arduino-cli compile -b lgt8fx:avr:328 .\BK1088_00_SERIAL_MONITOR\POC_03_SW --output-dir %USERPROFILE%\Downloads\hex\lgt8fx\POC_03_SW  

REM compiles ESP32 LCD16x2_ALL_IN_ONE
echo.
echo ********************
echo ESP32
echo ESP32 LCD16x2
echo ********************
arduino-cli compile --fqbn esp32:esp32:esp32-poe-iso .\BK1088_03_LCD16X02_ESP32 --output-dir %USERPROFILE%\Downloads\hex\ESP32\DEVM\BK1088_03_LCD16X02_ESP32  --warnings all

echo.
echo Compilation completed!
pause
