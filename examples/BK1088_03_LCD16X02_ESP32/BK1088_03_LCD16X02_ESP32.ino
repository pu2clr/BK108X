/*

  ESP32 Dev Modeule version.

  This sketch uses an ESP32 with LCD16X02 DISPLAY
  It is also a FM receiver capable to tune your local FM stations.
  This sketch saves the latest status of the receiver into the Atmega328 eeprom.

  TO RESET the EEPROM: Turn your receiver on with the encoder push button pressed.

  Wire up ESP32 Dev Module, BK1088E and LCD16x02 or LCD16x04

  | Device name               | Device Pin / Description  |  Arduino Pin  |
  | --------------------------| --------------------      | ------------  |
  |    LCD 16x2 or 20x4       |                           |               |
  |                           | D4                        |  GPIO18       |
  |                           | D5                        |  GPIO17       |
  |                           | D6                        |  GPIO16       |
  |                           | D7                        |  GPIO15       |
  |                           | RS                        |  GPIO19       |
  |                           | E/ENA                     |  GPIO23       |
  |                           | RW & VSS & K (16)         |  GND          |
  |                           | A (15) & VDD              |  +Vcc         |
  | --------------------------| ------------------------- | --------------|
  | BK1088E                   |                           |               | 
  |                           | VCC  (VD and VA)          |  3.3V         |
  |                           | SDIO / SDA (pin 8)        |  GPIO21       |
  |                           | SCLK (pin 7)              |  GPIO22       |
  | Buttons                   |                           |               |
  |                           | Volume Up                 |  GPIO32       |
  |                           | Volume Down               |  GPIO33       |
  |                           | Band Up                   |  GPIO25       |
  |                           | Band Down                 |  GPIO26       |
  | --------------------------| --------------------------| --------------|
  | Encoder                   |                           |               |
  |                           | A                         |  GPIO13       |
  |                           | B                         |  GPIO14       |
  |                           | PUSH BUTTON (encoder)     |  GPIO27       |
  | 


  By PU2CLR, Ricardo,  Feb  2023.
*/

#include <BK108X.h>
#include <EEPROM.h>
#include <LiquidCrystal.h>

#include "Rotary.h"

// LCD 16x02 or LCD20x4 PINs
#define LCD_D7 15
#define LCD_D6 16
#define LCD_D5 17
#define LCD_D4 18
#define LCD_RS 19
#define LCD_E 23

#define I2C_SDIO_PIN 21
#define I2C_SCLK_PIN 22

#define COLOR_BLACK 0x0000
#define COLOR_WHITE 0xFFFF

// Enconder PINs
#define ENCODER_PIN_A 13
#define ENCODER_PIN_B 14

// Buttons controllers
#define VOLUME_UP 32      // Volume Up
#define VOLUME_DOWN 33    // Volume Down
#define BAND_MODE_SWITCH_UP 25     // Next Band
#define BAND_MODE_SWITCH_DOWN 26  // Previous Band
#define SEEK_FUNCTION 27  //Encoder push button

#define DEFAULT_VOLUME_LEVEL 20

#define POLLING_TIME 10000
#define RDS_MSG_TYPE_TIME 25000
#define POLLING_RDS 40

#define STORE_TIME 10000  // Time of inactivity to make the current receiver status writable (10s / 10000 milliseconds).
#define PUSH_MIN_DELAY 300

#define EEPROM_SIZE 512
#define MIN_ELAPSED_TIME 150


/*
 * Structure used to refers a band / mode of the receiver
*/

typedef struct
{
  uint8_t  mode; // Band mode
  char    *name; // Band name 
  uint32_t minimum_frequency; // Minimum frequency to the band (KHz)
  uint32_t maximum_frequency; // Maximum frequency to the band (KHz)
  uint32_t default_frequency; // default frequency (KHz)
  uint16_t step;              // step used (KHz)
} tabBand;

/*
 * Table of the bands
 */ 

tabBand band[] = {
  {BK_MODE_FM, (char *) "FM", 8390, 10790, 10390, 20}, 
  {BK_MODE_AM, (char *) "MW ", 520, 1710, 810, 10},
  {BK_MODE_AM, (char *) "60m", 4700, 5600, 4885, 5},
  {BK_MODE_AM, (char *) "49m", 5700, 6400, 6100, 5},
  {BK_MODE_AM, (char *) "41m", 6800, 8200, 7205, 5},
  {BK_MODE_AM, (char *) "31m", 9200, 10500, 9600, 5},
  {BK_MODE_AM, (char *) "25m", 11400, 12200, 11940, 5},
  {BK_MODE_AM, (char *) "22m", 13400, 14300, 13600, 5},
  {BK_MODE_AM, (char *) "19m", 15000, 16100, 15300, 5},
  {BK_MODE_AM, (char *) "16m", 17400, 17900, 17600, 5},
  {BK_MODE_AM, (char *) "13m", 21400, 21900, 21525, 5}
};

const int lastBand = (sizeof band / sizeof(tabBand)) - 1;
int8_t bandIdx = 0; // FM

const uint8_t app_id = 88;  // Useful to check the EEPROM content before processing useful data
const int eeprom_address = 0;
long storeTime = millis();

bool bSt = true;

uint8_t seekDirection = 1;  // 0 = Down; 1 = Up. This value is set by the last encoder direction.

long pollin_elapsed = millis();

int maxX1;
int maxY1;

// Encoder control variables
volatile int encoderCount = 0;
uint16_t currentFrequency;
uint16_t previousFrequency;

// Encoder control
Rotary encoder = Rotary(ENCODER_PIN_A, ENCODER_PIN_B);

// LCD display
LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
BK108X rx;

void setup() {

  pinMode(ENCODER_PIN_A, INPUT_PULLUP);
  pinMode(ENCODER_PIN_B, INPUT_PULLUP);

  // Push button pin
  pinMode(VOLUME_UP, INPUT_PULLUP);
  pinMode(VOLUME_DOWN, INPUT_PULLUP);
  pinMode(SEEK_FUNCTION, INPUT_PULLUP);
  pinMode(BAND_MODE_SWITCH_UP, INPUT_PULLUP);
  pinMode(BAND_MODE_SWITCH_DOWN, INPUT_PULLUP);

  // Start LCD display device
  lcd.begin(16, 2);
  showSplash();

  EEPROM.begin(EEPROM_SIZE);

  // If you want to reset the eeprom, keep the ENCODER PUSH BUTTON  pressed during statup
  if (digitalRead(SEEK_FUNCTION) == LOW) {
    EEPROM.write(eeprom_address, 0);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("RESET");
    delay(1500);
    showSplash();
  }

  // Encoder interrupt
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), rotaryEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_B), rotaryEncoder, CHANGE);

  rx.setup(I2C_SDIO_PIN, I2C_SCLK_PIN);
  delay(100);

  // Checking the EEPROM content
  if (EEPROM.read(eeprom_address) == app_id) {
    readAllReceiverInformation();
  } else {
    // Default values
    rx.setMono(false);  // Force stereo
    // rx.setRBDS(true);  //  set RDS and RBDS. See setRDS.
    // rx.setRDS(true);
    // rx.RdssetRdsFifo(true);
    useBand();
    rx.setVolume(DEFAULT_VOLUME_LEVEL);
  }
  rx.setRDS(true);
  lcd.clear();
  showStatus();
}


void saveAllReceiverInformation() {
  EEPROM.begin(EEPROM_SIZE);

  // The write function/method writes data only if the current data is not equal to the stored data.
  EEPROM.write(eeprom_address, app_id);
  EEPROM.write(eeprom_address + 1, rx.getVolume());           // stores the current Volume
  EEPROM.write(eeprom_address + 2, currentFrequency >> 8);    // stores the current Frequency HIGH byte for the band
  EEPROM.write(eeprom_address + 3, currentFrequency & 0xFF);  // stores the current Frequency LOW byte for the band
  EEPROM.write(eeprom_address + 5, (uint8_t)bSt);
  EEPROM.write(eeprom_address + 6, bandIdx);

  EEPROM.end();
}

void readAllReceiverInformation() {
  rx.setVolume(EEPROM.read(eeprom_address + 1));
  currentFrequency = EEPROM.read(eeprom_address + 2) << 8;
  currentFrequency |= EEPROM.read(eeprom_address + 3);
  previousFrequency = currentFrequency;


  bSt = (bool)EEPROM.read(eeprom_address + 5);
  bandIdx = EEPROM.read(eeprom_address + 6);
  band[bandIdx].default_frequency = currentFrequency;
  useBand();
  rx.setMono(bSt);
}


/*
   To store any change into the EEPROM, it is needed at least STORE_TIME  milliseconds of inactivity.
*/
void resetEepromDelay() {
  delay(PUSH_MIN_DELAY);
  storeTime = millis();
  previousFrequency = 0;
}


/*
    Reads encoder via interrupt
    Use Rotary.h and  Rotary.cpp implementation to process encoder via interrupt
*/
void rotaryEncoder() {  // rotary encoder events
  uint8_t encoderStatus = encoder.process();
  if (encoderStatus)
    encoderCount = (encoderStatus == DIR_CW) ? 1 : -1;
}

void showSplash() {
  lcd.setCursor(0, 0);
  lcd.print("PU2CLR-BK1088");
  lcd.setCursor(0, 1);
  lcd.print("Arduino Library");
  lcd.display();
  delay(1000);
}

/*
   Shows the static content on  display
*/
void showTemplate() {
}


/*
   Shows frequency information on Display
*/
void showFrequency() {
  currentFrequency = rx.getFrequency();
  lcd.setCursor(4, 1);
  if ( band[bandIdx].mode == BK_MODE_FM)
     lcd.print(rx.formatCurrentFrequency());
  else {
      if ( currentFrequency < 1000) 
        lcd.print(rx.formatCurrentFrequency(' ',0));
      else   
        lcd.print(rx.formatCurrentFrequency('.',2));
  }
  lcd.display();
}

void showFrequencySeek() {
  clearLcdLine(1);
  showFrequency();
}

/*
    Show some basic information on display
*/
void showStatus() {
  clearLcdLine(1);
  showFrequency();
  showBandStatus();
  showRSSI();

  lcd.display();
}

/* *******************************
   Shows RSSI status
*/
void showRSSI() {
  char rssi[12];
  rx.convertToChar( rx.getRssi(), rssi, 3, 0, '.');
  strcat(rssi, "dB");
  lcd.setCursor(13, 1);
  lcd.print(rssi);
}

void showBandStatus() {
  lcd.setCursor(0, 1);
  lcd.print(band[bandIdx].name);
}

void bandUp()
{
  // save the current frequency for the band
  band[bandIdx].default_frequency = currentFrequency;

  if (bandIdx < lastBand)
  {
    bandIdx++;
  }
  else
  {
    bandIdx = 0;
  }
  useBand();
}

void bandDown()
{
  // save the current frequency for the band
  band[bandIdx].default_frequency = currentFrequency;

  if (bandIdx > 0)
  {
    bandIdx--;
  }
  else
  {
    bandIdx = lastBand;
  }
  useBand();
}


void useBand() {

  if (band[bandIdx].mode ==  BK_MODE_FM)
  {
    rx.setFM(band[bandIdx].minimum_frequency, band[bandIdx].maximum_frequency, band[bandIdx].default_frequency, band[bandIdx].step);
    rx.setRDS(true);
    rx.setFmDeemphasis(DE_EMPHASIS_75);
    // rx.setSoftMute(true);
    rx.setMono(false);  // Force stereo
  }
  else
  {
    rx.setAM(band[bandIdx].minimum_frequency, band[bandIdx].maximum_frequency, band[bandIdx].default_frequency, band[bandIdx].step);
  }
  delay(100);
  currentFrequency = band[bandIdx].default_frequency;
  rx.setFrequency(currentFrequency);
  delay(MIN_ELAPSED_TIME); // waits a little more for releasing the button.
  showStatus();
}

/*********************************************************
   RDS
 *********************************************************/
char *programInfo;
char *localTime;
long polling_rds = millis();

int progInfoIndex = 0;  // controls the part of the rdsMsg text will be shown on LCD 16x2 Display

long delayProgramInfo = millis();
long delayLocalTime = millis();
long waitTime = 1000L;

/**
  showProgramInfo - Shows the Program Information
*/
void showProgramInfo() {
  char txtAux[17];

  if (programInfo == NULL || strlen(programInfo) < 2 || (millis() - delayProgramInfo) < waitTime) return;
  delayProgramInfo = millis();
  clearLcdLine(0);
  programInfo[61] = '\0';  // Truncate the message to fit on display line
  strncpy(txtAux, &programInfo[progInfoIndex], 16);
  txtAux[16] = '\0';
  progInfoIndex += 3;
  if (progInfoIndex > 60) progInfoIndex = 0;
  lcd.setCursor(0, 0);
  lcd.print(txtAux);
  waitTime = 1000L;
}


void showTime() {
  if (localTime == NULL || strlen(localTime) < 4 || (millis() - delayLocalTime) < 50000) return;
  clearLcdLine(0);
  lcd.setCursor(0, 0);
  lcd.print(localTime);
  delayProgramInfo = millis(); // Stop showing Program Information for 10s
  delayLocalTime = millis(); 
  waitTime = 10000L;
} 

void clearLcdLine(uint8_t line) {
  lcd.setCursor(0,line);
  lcd.print("                ");
}

void clearRds() {
  programInfo = NULL;
  rx.clearRdsBuffer();
  clearLcdLine(0);
}

void checkRDS() {
  // You must call getRdsReady before calling any RDS query function.
  if (rx.getRdsReady()) {
      programInfo = rx.getRdsProgramInformation();
      localTime = rx.getRdsLocalTime();
      showProgramInfo();
      showTime();
  }
}

/*********************************************************

 *********************************************************/


void doStereo() {
  rx.setMono((bSt = !bSt));
  showBandStatus();
  resetEepromDelay();
}

/**
   Process seek command.
   The seek direction is based on the last encoder direction rotation.
*/
void doSeek() {
  rx.seek(BK_SEEK_WRAP, seekDirection, showFrequencySeek);  // showFrequency will be called by the seek function during the process.
  delay(200);
  showStatus();
}

void loop() {

  // Check if the encoder has moved.
  if (encoderCount != 0) {
    if (encoderCount == 1) {
      rx.setFrequencyUp();
      seekDirection = BK_SEEK_UP;
    } else {
      rx.setFrequencyDown();
      seekDirection = BK_SEEK_DOWN;
    }
    showStatus();
    encoderCount = 0;
    storeTime = millis();
  }

  if (digitalRead(VOLUME_UP) == LOW) {
    rx.setVolumeUp();
    resetEepromDelay();
  } else if (digitalRead(VOLUME_DOWN) == LOW) {
    rx.setVolumeDown();
    resetEepromDelay();
  } else if (digitalRead(SEEK_FUNCTION) == LOW)
    doStereo();
  else if (digitalRead(BAND_MODE_SWITCH_UP) == LOW)
    bandUp();
  else if (digitalRead(BAND_MODE_SWITCH_DOWN) == LOW)
    bandDown();
  if ((millis() - pollin_elapsed) > POLLING_TIME) {
    showStatus();
    pollin_elapsed = millis();
  }

  if ((millis() - polling_rds) > POLLING_RDS && band[bandIdx].mode == BK_MODE_FM)  {
      checkRDS();
    polling_rds = millis();
  }


  // Show the current frequency only if it has changed
  if ((currentFrequency = rx.getFrequency()) != previousFrequency) {
    clearRds();
    if ((millis() - storeTime) > STORE_TIME) {
      saveAllReceiverInformation();
      storeTime = millis();
      previousFrequency = currentFrequency;
    }
  }

  delay(5);
}
