@echo off
REM Quick setup script for users who already have arduino-cli installed
REM This script only installs the required libraries and boards for BK108X examples
REM 
REM Prerequisites: arduino-cli must be already installed and available in PATH
REM
REM Ricardo Lima Caratti - Windows version

echo ====================================================
echo BK108X Libraries and Boards Quick Setup
echo ====================================================
echo.
echo This script assumes arduino-cli is already installed.
echo It will only install libraries and board packages.
echo.
pause

REM Check if arduino-cli is available
arduino-cli version >nul 2>&1
if errorlevel 1 (
    echo ERROR: arduino-cli not found in PATH
    echo Please install arduino-cli first or use config_libraries_and_boards.bat
    pause
    exit /b 1
)

echo arduino-cli found. Proceeding with setup...
echo.

REM Update board index
echo Updating board index...
arduino-cli core update-index

echo.
echo Installing required libraries...
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

echo.
echo Installing board packages...
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
echo Quick setup completed successfully!
echo ====================================================
echo.
echo You can now use compile_all.bat to compile the examples.
echo.
pause
