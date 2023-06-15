/*
  This sketch uses an Arduino Nano with NOKIA 5110 display. 
  It is a FM receiver with RDS feature capable to tune your local FM stations.
  This sketch saves the latest status of the receiver into the Atmega328 eeprom (frequency, RDS and Stereo setup).
  
  ABOUT THE EEPROM:
  ATMEL says the lifetime of an EEPROM memory position is about 100,000 writes.
  For this reason, this sketch tries to avoid unnecessary writes into the eeprom.
  So, the condition to store any status of the receiver is changing the frequency or volume and 10 seconds of inactivity.
  For example, if you switch the band and turn the receiver off immediately, no new information will be written into the eeprom.
  But you wait 10 seconds after changing anything, all new information will be written.

  TO RESET the EEPROM: Turn your receiver on with the encoder push button pressed.


  Read more on https://pu2clr.github.io/BK1088/

  Wire up on Arduino UNO, Nano or Pro mini

  | Device name               | Device Pin / Description  |  Arduino Pin  |
  | --------------------------| --------------------      | ------------  |
  | NOKIA 5110                |                           |               |
  |                           | (1) RST (RESET)           |     8         |
  |                           | (2) CE or CS              |     9         |
  |                           | (3) DC or DO              |    10         |
  |                           | (4) DIN or DI or MOSI     |    11         |
  |                           | (5) CLK                   |    13         |
  |                           | (6) VCC  (3V-5V)          |    +VCC       |
  |                           | (7) BL/DL/LIGHT           |    +VCC       |
  |                           | (8) GND                   |    GND        |
  | --------------------------| ------------------------- | --------------|
  | BK1088                    |                           |               | 
  |                           | +Vcc 3.3V [1]             |   3.3V        |    
  |                           | SDIO (pin 8)              |     A4        |
  |                           | SCLK (pin 7)              |     A5        |
  | --------------------------| --------------------------| --------------|
  | Buttons                   |                           |               |
  |                           | Volume Up                 |      4        |
  |                           | Volume Down               |      5        |
  |                           | Stereo/Mono               |      6        |
  |                           | RDS ON/off                |      7        |
  |                           | SEEK (encoder button)     |     A0/14     |
  | --------------------------| --------------------------|---------------| 
  | Encoder                   |                           |               |
  |                           | A                         |       2       |
  |                           | B                         |       3       |

  1. Do not use more than 3.3V to power the BK1088.

  Please, see user_manual.txt

  Prototype documentation: https://pu2clr.github.io/BK1088/
  PU2CLR BK1088 API documentation: https://pu2clr.github.io/BK1088/extras/apidoc/html/

  By PU2CLR, Ricardo,  Feb  2023.
*/

#include <BK108X.h>
#include <EEPROM.h>

#include <Adafruit_GFX.h>      // Core graphics library
#include <Adafruit_PCD8544.h>  // See: https://www.electronoobs.com/eng_arduino_Adafruit_PCD8544.php
#include <SPI.h>

#include "Rotary.h"

// NOKIA Display pin setup
#define NOKIA_RST 8   // RESET
#define NOKIA_CE 9    // Some NOKIA devices show CS
#define NOKIA_DC 10   //
#define NOKIA_DIN 11  // MOSI
#define NOKIA_CLK 13  // SCK
#define NOKIA_LED 0   // 0 if wired to +3.3V directly


#define SDA_PIN A4 // SDA pin used by your Arduino Board
#define CLK_PIN A5


#define COLOR_BLACK 0x0000
#define COLOR_WHITE 0xFFFF

// Enconder PINs
#define ENCODER_PIN_A 2
#define ENCODER_PIN_B 3

// Buttons controllers
#define VOLUME_UP 4       // Volume Up
#define VOLUME_DOWN 5     // Volume Down
#define BAND_UP 6   // Select Mono or Stereo
#define BAND_DOWN 7      // SDR ON or OFF
#define SEEK_FUNCTION 14  // Pin A0 / Digital 14

#define POLLING_TIME 3000
#define MIN_ELAPSED_TIME 200

#define STORE_TIME 10000    // Time of inactivity to make the current receiver status writable (10s / 10000 milliseconds).
#define PUSH_MIN_DELAY 300  // Minimum waiting time after an action
#define POLLING_RDS 40
#define DEFAULT_VOLUME 25

const uint8_t app_id = 88;  // Useful to check the EEPROM content before processing useful data
const int eeprom_address = 0;
long storeTime = millis();


uint8_t seekDirection = 1;  // 0 = Down; 1 = Up. This value is set by the last encoder direction.

long pollin_elapsed = millis();

int maxX1;
int maxY1;

// Encoder control variables
volatile int encoderCount = 0;
uint16_t currentFrequency;
uint16_t previousFrequency;


typedef struct
{
  uint8_t  mode; // Bande mode.
  char    *name;  
  uint32_t minimum_frequency; // Minimum frequency to the band (KHz)
  uint32_t maximum_frequency; // Maximum frequency to the band (KHz)
  uint32_t default_frequency; // default frequency (KHz)
  uint16_t step;               // step used (KHz)
  uint8_t band_space;           // AM: (0=1KHz; 1 = 5KHz; 2=9KHz; 3 = 10KHz) | FM: Not used here.
} tabBand;

tabBand band[] = {
  {BK_MODE_FM, (char *) "FM", 6400, 10800, 10390, 10, 2},
  {BK_MODE_AM, (char *) "MW1", 520, 1710, 810, 10, 3},    //
  {BK_MODE_AM, (char *) "MW2", 522, 1710, 810,  9, 2},    // MW/AM - Europe
  {BK_MODE_AM, (char *) "60m", 4700, 5600, 4885, 5, 1},
  {BK_MODE_AM, (char *) "49m", 5700, 6400, 6100, 5, 1},
  {BK_MODE_AM, (char *) "41m", 6800, 8200, 7205, 5, 1},
  {BK_MODE_AM, (char *) "31m", 9200, 10500, 9600, 5, 1},
  {BK_MODE_AM, (char *) "25m", 11400, 12200, 11940, 5, 1},
  {BK_MODE_AM, (char *) "22m", 13400, 14300, 13600, 5, 1},
  {BK_MODE_AM, (char *) "19m", 15000, 16100, 15300, 5, 1},
  {BK_MODE_AM, (char *) "16m", 17400, 17900, 17600, 5, 1},
  {BK_MODE_AM, (char *) "13m", 21400, 21900, 21525, 5, 1}
};

const int lastBand = (sizeof band / sizeof(tabBand)) - 1;
int bandIdx = 0; // FM


// Encoder control
Rotary encoder = Rotary(ENCODER_PIN_A, ENCODER_PIN_B);

// Nokia 5110 display
Adafruit_PCD8544 display = Adafruit_PCD8544(NOKIA_DC, NOKIA_CE, NOKIA_RST);

BK108X rx;

void setup() {

  pinMode(ENCODER_PIN_A, INPUT_PULLUP);
  pinMode(ENCODER_PIN_B, INPUT_PULLUP);

  // Push button pin
  pinMode(VOLUME_UP, INPUT_PULLUP);
  pinMode(VOLUME_DOWN, INPUT_PULLUP);
  pinMode(BAND_UP, INPUT_PULLUP);
  pinMode(BAND_DOWN, INPUT_PULLUP);
  pinMode(SEEK_FUNCTION, INPUT_PULLUP);

  // Start the Nokia display device
  display.begin();
  // ATTENTION: YOU MUST VERIFY THE BEST LAVEL FOR THE CONTRAST OF YOUR DISPLAY.
  display.setContrast(60);  // You may need adjust this value for you Nokia 5110
  showSplash();
  showTemplate();


  // If you want to reset the eeprom, keep the ENCODER PUSH BUTTON  pressed during statup
  if (digitalRead(SEEK_FUNCTION) == LOW) {
    EEPROM.write(eeprom_address, 0);
    display.clearDisplay();
    display.display();
    display.setTextColor(BLACK);
    display.setTextSize(2);
    display.setCursor(0, 10);
    display.print("RESET");
    display.display();
    delay(1500);
    showSplash();
  }

  // Encoder interrupt
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), rotaryEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_B), rotaryEncoder, CHANGE);

  rx.setup(SDA_PIN,CLK_PIN); 
  // rx.setup(SDA_PIN, CLK_PIN, OSCILLATOR_TYPE_REFCLK, 12000000); // 12MHz external clock oscillator

  // Checking the EEPROM content
  if (EEPROM.read(eeprom_address) == app_id) {
    readAllReceiverInformation();
  } else {
    // Default values
    rx.setVolume(DEFAULT_VOLUME);
    rx.setMono(false);  // Force stereo
    // rx.setRBDS(true);  //  set RDS and RBDS. See setRDS.
    rx.setRDS(true);
    currentFrequency = previousFrequency = band[bandIdx].default_frequency;
    useBand();
  }

  
  // rx.setFrequency(currentFrequency);  // It is the frequency you want to select in MHz multiplied by 100.

  showStatus();
}


void saveAllReceiverInformation() {
  EEPROM.update(eeprom_address, app_id);
  EEPROM.update(eeprom_address + 1, rx.getVolume());           // stores the current Volume
  EEPROM.update(eeprom_address + 2, currentFrequency >> 8);    // stores the current Frequency HIGH byte for the band
  EEPROM.update(eeprom_address + 3, currentFrequency & 0xFF);  // stores the current Frequency LOW byte for the band
  EEPROM.update(eeprom_address + 6, bandIdx);
}

void readAllReceiverInformation() {
  rx.setVolume(EEPROM.read(eeprom_address + 1));
  currentFrequency = EEPROM.read(eeprom_address + 2) << 8;
  currentFrequency |= EEPROM.read(eeprom_address + 3);
  previousFrequency = currentFrequency;

  bandIdx = EEPROM.read(eeprom_address + 6);
  band[bandIdx].default_frequency = currentFrequency;
  useBand();
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
  display.clearDisplay();
  display.display();
  display.setTextColor(BLACK);
  // Splash - Change it by the your introduction text.
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.print("BK1088");
  display.setCursor(0, 15);
  display.print("Arduino");
  display.setCursor(0, 30);
  display.print("Library");
  display.display();
  delay(2000);
  display.clearDisplay();
  display.display();
}

/*
   Shows the static content on  display
*/
void showTemplate() {

  maxX1 = display.width() - 2;
  maxY1 = display.height() - 2;

  // TO DO: The frame of the screen
}


/*
   Shows frequency information on Display
*/
void showFrequency() {
  currentFrequency = rx.getFrequency();
  display.setCursor(3, 8);
  display.setTextSize(2);
  if ( band[bandIdx].mode == BK_MODE_FM)
     display.print(rx.formatCurrentFrequency());
  else {
      if ( currentFrequency < 1000) 
        display.print(rx.formatCurrentFrequency(' ',0));
      else   
        display.print(rx.formatCurrentFrequency('.',2));
  }
  display.display();
}

void showFrequencySeek() {
  display.clearDisplay();
  showFrequency();
  display.display();
}

/*
    Show some basic information on display
*/
void showStatus() {
  display.fillRect(0, 0, 84, 23, WHITE);
  display.setTextColor(BLACK);
  showFrequency();
  showRSSI();
  showBandName();
  display.display();
}

/* *******************************
   Shows RSSI status
*/
void showRSSI() {
  char rssi[12];
  display.setTextSize(1);
  rx.convertToChar(rx.getRssi(), rssi, 3, 0, '.');
  strcat(rssi, "dB");
  display.setCursor(53, 0);
  display.print(rssi);
}

void showBandName() {
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(band[bandIdx].name);
}


/*********************************************************
   RDS
 *********************************************************/
char *programInfo;
char *stationName;
char *rdsTime;

long delayStationName = millis();
long delayProgramInfo = millis();
long delayTime = millis();
long rds_pulling = millis();
uint8_t idxProgramInfo = 0;

void showProgramInfo() {

  char aux[20];

  if (programInfo == NULL || (strlen(programInfo) < 2) || (millis() - delayProgramInfo) < 1000) return;

  strncpy(aux, &programInfo[idxProgramInfo], 14);
  aux[14] = '\0';
  display.setTextSize(1);
  display.fillRect(0, 24, 84, 8, WHITE);
  display.setCursor(0, 24);
  display.print(aux);
  idxProgramInfo += 4;
  if (idxProgramInfo > 60) idxProgramInfo = 0;
  display.display();
  delayProgramInfo = millis();
}

/**
   TODO: process RDS Dynamic PS or Scrolling PS
*/
void showRDSStation() {

  if (stationName == NULL || strlen(stationName) < 2 || (millis() - delayStationName) < 6000) return;
  display.setTextSize(1);
  display.fillRect(0, 40, 49, 8, WHITE);
  display.setCursor(0, 40);
  stationName[8] = 0;
  display.print(stationName);
  display.display();
  delayStationName = millis();
}

void showRDSTime() {
  char *p;
  if (rdsTime == NULL || (millis() - delayTime) < 60000) return;

  // Shows also the current program type.
  display.fillRect(0, 32, 84, 8, WHITE);
  display.setCursor(0, 32);
  display.print(rx.getRdsProgramType());
  switch (rx.getRdsProgramType()) {
    case 1: p = (char *) "News"; break;
    case 3:
    case 4: p = (char *) "Info/Sport"; break;
    case 7: p = (char *) "Culture"; break;
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15: p = (char *) "Music"; break;
    default: p = (char *) "Other";
  }
  display.setCursor(15, 32);
  display.print(p);

  display.fillRect(42, 40, 42, 8, WHITE);
  display.setCursor(50, 40);
  display.print(rdsTime);
  display.display();
  delayTime = millis();
}


void clearRds() {
  programInfo = NULL;
  stationName = NULL;
  rdsTime = NULL;
  rx.clearRdsBuffer();
  display.fillRect(0, 24, 84, 24, WHITE);
}

void checkRDS() {
  // You must call getRdsReady before calling any RDS query function.
  if (rx.getRdsReady()) {
      programInfo = rx.getRdsProgramInformation();
      stationName = rx.getRdsStationName();
      rdsTime = rx.getRdsLocalTime();  // Gets the Local Time. Check the getRdsTime documentation for more details. Some stations do not broadcast the right time.
      showProgramInfo();
      showRDSStation();
      showRDSTime();
  }
}


/*********************************************************

 *********************************************************/

/*
 * Selects the next band (see bandTable)
 */
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

/*
 * Selects the previous band (See bandTable).
 */
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
    rx.setFMGain(0); // +18db
    rx.setFmDeemphasis(DE_EMPHASIS_75);
    rx.setMono(false);  // Force stereo
  }
  else
  {
    rx.setAM(band[bandIdx].minimum_frequency, band[bandIdx].maximum_frequency, band[bandIdx].default_frequency, band[bandIdx].step, band[bandIdx].band_space);
  }
  delay(100);
  currentFrequency = band[bandIdx].default_frequency;
  rx.setFrequency(currentFrequency);
  delay(MIN_ELAPSED_TIME); // waits a little more for releasing the button.
  showStatus();
}

/**
   Process seek command.
   The seek direction is based on the last encoder direction rotation.
*/
void doSeek() {
       
  rx.seek(BK_SEEK_WRAP, seekDirection, showFrequencySeek);  // showFrequency will be called by the seek function during the process.
  delay(PUSH_MIN_DELAY);
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
  } else if (digitalRead(BAND_UP) == LOW) {
    bandUp();
  }
  else if (digitalRead(BAND_DOWN) == LOW) {
    bandDown();
  }
  else if (digitalRead(SEEK_FUNCTION) == LOW)
    doSeek();

  if ((millis() - pollin_elapsed) > POLLING_TIME) {
    showStatus();
    pollin_elapsed = millis();
  }

  if ( (millis() - rds_pulling) > POLLING_RDS  && band[bandIdx].mode == BK_MODE_FM)   {
     checkRDS();
     rds_pulling = millis();
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
