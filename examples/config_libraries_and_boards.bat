@echo off
REM This script runs on Windows and it is useful to install the basic environment to develop
REM applications (receivers) using the PU2CLR Arduino Library.
REM
REM ATTENTION: Please, do not run it if you do not know what you are doing.
REM
REM This script will:
REM 1. Download and install arduino-cli
REM 2. Configure board manager URLs
REM 3. Install required libraries
REM 4. Install board packages
REM
REM Please, check the comments above
REM
REM Ricardo Lima Caratti - Windows version

echo ====================================================
echo PU2CLR BK108X Arduino Environment Setup for Windows
echo ====================================================
echo.
echo This script will install arduino-cli and configure
echo the development environment for BK108X library.
echo.
echo WARNING: This may take several minutes or hours depending
echo on your internet connection. Please wait!
echo.
pause

REM Create bin directory in user profile if it doesn't exist
if not exist "%USERPROFILE%\bin" mkdir "%USERPROFILE%\bin"

REM Download arduino-cli (Windows version)
echo Downloading arduino-cli for Windows...
powershell -Command "& {[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; Invoke-WebRequest -Uri 'https://downloads.arduino.cc/arduino-cli/arduino-cli_latest_Windows_64bit.zip' -OutFile '%TEMP%\arduino-cli.zip'}"

REM Extract arduino-cli
echo Extracting arduino-cli...
powershell -Command "& {Expand-Archive -Path '%TEMP%\arduino-cli.zip' -DestinationPath '%USERPROFILE%\bin' -Force}"

REM Add to PATH (temporary for this session)
set PATH=%USERPROFILE%\bin;%PATH%

REM Start the arduino-cli configuration
echo Initializing arduino-cli configuration...
arduino-cli config init

REM Add all boards used by the examples (ATmega, Attiny, ESP32, STM32 etc) to the board list
echo Configuring board manager URLs...
arduino-cli config set board_manager.additional_urls ^
http://arduino.esp8266.com/stable/package_esp8266com_index.json,^
http://dan.drown.org/stm32duino/package_STM32duino_index.json,^
http://drazzy.com/package_drazzy.com_index.json,^
https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json,^
https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json,^
https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json,^
https://github.com/stm32duino/BoardManagerFiles/raw/master/STM32/package_stm_index.json,^
https://mcudude.github.io/MegaCore/package_MCUdude_MegaCore_index.json,^
https://mcudude.github.io/MightyCore/package_MCUdude_MightyCore_index.json,^
https://mcudude.github.io/MiniCore/package_MCUdude_MiniCore_index.json,^
https://raw.githubusercontent.com/DavidGuo-CS/OSOYOO_Arduino/main/package_osoyoo_boards_index.json,^
https://raw.githubusercontent.com/VSChina/azureiotdevkit_tools/master/package_azureboard_index.json,^
https://raw.githubusercontent.com/damellis/attiny/ide-1.6.x-boards-manager/package_damellis_attiny_index.json,^
https://raw.githubusercontent.com/dbuezas/lgt8fx/master/package_lgt8fx_index.json,^
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json,^
https://raw.githubusercontent.com/nulllaborg/arduino_nulllab/master/package_nulllab_boards_index.json,^
https://www.pjrc.com/teensy/package_teensy_index.json

REM Update the index of boards that can be installed
echo Updating board index...
arduino-cli core update-index

echo.
echo Installing the libraries used by the examples implemented in this Library...
echo This may take several minutes. Please wait!

REM Install libraries
arduino-cli lib install "PU2CLR BK108X"
arduino-cli lib install "Adafruit BusIO"
arduino-cli lib install "Adafruit SH110X"
arduino-cli lib install "Adafruit SSD1306"
arduino-cli lib install "TM1638lite"
arduino-cli lib install "Adafruit ST7735 and ST7789 Library"
arduino-cli lib install "Adafruit PCD8544 Nokia 5110 LCD library"
arduino-cli lib install "LiquidCrystal"
arduino-cli lib install "MCUFRIEND_kbv"
arduino-cli lib install "TFT_22_ILI9225"
arduino-cli lib install "Tiny4kOLED"
arduino-cli lib install "TinyOLED-Fonts"
arduino-cli lib install "FastLED"
arduino-cli lib install "RotaryEncoder"

echo ----------------------------------------------------
echo Installing additional libraries via github repository...
set ARDUINO_LIBRARY_ENABLE_UNSAFE_INSTALL=true
arduino-cli lib install --git-url https://github.com/wonho-maker/Adafruit_SH1106 
arduino-cli lib install --git-url https://github.com/danilopinotti/Battery18650Stats 
echo ----------------------------------------------------

echo.
echo Installing the boards...
arduino-cli core install arduino:avr
arduino-cli core install lgt8fx:avr
arduino-cli core install MiniCore:avr
arduino-cli core install arduino:sam
arduino-cli core install esp32:esp32
arduino-cli core install esp8266:esp8266
arduino-cli core install stm32duino:STM32F1
arduino-cli core install ATTinyCore:avr

echo.
echo ====================================================
echo Installation completed successfully!
echo ====================================================
echo.
echo IMPORTANT: To use arduino-cli in future sessions, 
echo add %USERPROFILE%\bin to your Windows PATH environment variable.
echo.
echo You can now use compile_all.bat to compile the examples.
echo.
pause
