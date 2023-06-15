/*
  This sketch runs on ESP32 device LilyGO T-Embed panel.
  
  The interface design of the sketch used here was written by Volos (https://github.com/VolosR/TEmbedFMRadio). 
  The Volos' project is a receiver based on the TEA5767 device and I have adapted it to the BK1088 device.

  Please, read the file user_manual.txt or README.md files for more details. 

  The  purpose  of  this  example  is  to  demonstrate a prototype  receiver based  on  the  BK1088E and  the
  "PU2CLR BK108X Arduino Library". It is not the purpose of this prototype  to provide you a beautiful interface. You can do it better.

  ESP32 and components wire up.

  | Device name            | Device Pin / Description     |  ESP32    |
  | -----------------------| -----------------------------| ----------|
  | LilyGo Painel I2C      |                              |           |
  |                        | SDA/SDIO                     |  GPIO 18  |
  |                        | SCL/SCLK                     |  GPIO  8  |
  | Encoder  LilyGo Painel |                              |           |
  |                        | A                            |  GPIO  2  |
  |                        | B                            |  GPIO  1  |
  |                        | PUSH BUTTON (encoder)        |  GPIO  0  |
  |                        |                              |           |  
  | Battery Indicator      |                              |  GPIO  4  |
  | LCD LED                |                              |  GPIO 15  |  
  | Power ON               |                              |  GPIO 46  |
  |                        |                              |           |
  | SI453X MCU RESET PIN   |                              |           |
  |  See table below       | ESP32 PIN USED FOR RESETTING |  GPIO 16  |  



  ### ESP32-S3 and BK1088E wire up

  | BK1088E |  DESC.  | ESP32    (GPIO)    |
  |---------| --------|--------------------|
  | pin  8  |  SDIO   | 18 (SDA / GPIO18)  |
  | pin  7  |  SCLK   |  8 (SCL / GPIO8)   |

  If you are using the SI4732-A10, check the corresponding pin numbers.

  By PU2CLR, Ricardo, Dec  2022.
*/

#include "EEPROM.h"
#include <BK108X.h>
#include <FastLED.h>
#include <TFT_eSPI.h>
#include <Battery18650Stats.h> // Install it from: https://github.com/danilopinotti/Battery18650Stats

#include "Rotary.h"

#define POLLING_RDS 40

// Enconder PINs
#define ENCODER_PIN_A  1           // GPIO2 
#define ENCODER_PIN_B  2           // GPIO1

// I2C bus pin on ESP32
#define ESP32_I2C_SDA 18
#define ESP32_I2C_SCL 8

#define PIN_BAT_VOLT   4
#define PIN_POWER_ON  46
#define PIN_LCD_BL    15 

// Buttons controllers
#define ENCODER_PUSH_BUTTON 0     // GPIO0

#define MIN_ELAPSED_TIME 300
#define MIN_ELAPSED_RSSI_TIME 200
#define ELAPSED_COMMAND 2000  // time to turn off the last command controlled by encoder. Time to goes back to the FVO control
#define ELAPSED_CLICK 1500    // time to check the double click commands
#define DEFAULT_VOLUME 25     // change it for your favorite sound volume
#define ELAPSED_TIME_BATERRY 60000

#define EEPROM_SIZE        512

#define STORE_TIME 10000 // Time of inactivity to make the current receiver status writable (10s / 10000 milliseconds).

#define NUM_LEDS 7
#define DATA_PIN 42
#define CLOCK_PIN 45

#define color1 0xC638
#define color2 0xC638

Battery18650Stats battery(PIN_BAT_VOLT);

// EEPROM - Stroring control variables
const uint8_t app_id = 86; // Useful to check the EEPROM content before processing useful data
const int eeprom_address = 0;
long storeTime = millis();

bool itIsTimeToSave = false;
uint8_t countClick = 0;
uint8_t seekDirection = 1;

bool cmdBand = false;
bool cmdVolume = false;
bool cmdStep = false;
bool cmdMode = false;
bool cmdMenu = false;
bool cmdSoftMuteMaxAtt = false;

long elapsedRSSI = millis();
long elapsedButton = millis();
long elapsedBattery = millis();
long elapsedClick = millis();
long elapsedCommand = millis();
volatile int encoderCount = 0;
uint16_t currentFrequency;

const char *menu[] = {"Volume", "Step", "SoftMute", "Seek"};
int8_t menuIdx = 0;
const int lastMenu = 3;
int8_t currentMenuCmd = -1;

/*
 * Structure used to refers a band / mode of the receiver
*/

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
  {BK_MODE_FM, (char *) "FM", 6400, 10800, 10390, 10, 2}, // 100kHz - Band space
  {BK_MODE_AM, (char *) "MW1", 520, 1710, 810, 10, 3},    // 10 kHz
  {BK_MODE_AM, (char *) "MW2", 522, 1710, 810,  9, 2},    // MW/AM - Europe - 9kHz
  {BK_MODE_AM, (char *) "60m", 4700, 5600, 4885, 5, 1},   // Always 5 kHz
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
int8_t bandIdx = 0; // FM

uint8_t rssi = 0;
uint8_t volume = DEFAULT_VOLUME;

CRGB leds[NUM_LEDS];

// Devices class declarations
Rotary encoder = Rotary(ENCODER_PIN_A, ENCODER_PIN_B);

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);

BK108X rx;

void setup()
{
  // Encoder pins
  pinMode(ENCODER_PUSH_BUTTON, INPUT_PULLUP);
  pinMode(ENCODER_PIN_A, INPUT_PULLUP);
  pinMode(ENCODER_PIN_B, INPUT_PULLUP);

  FastLED.addLeds<APA102, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);

  tft.begin();
  tft.writecommand(0x11);
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  
  turnDisplay(true);

  spr.createSprite(320, 170);
  spr.setTextDatum(4);
  spr.setSwapBytes(true);
  spr.setFreeFont(&Orbitron_Light_24);
  spr.setTextColor(color1, TFT_BLACK);

  // Splash - Remove or change it for your own introduction.
  splash(); 

  EEPROM.begin(EEPROM_SIZE);

  // If you want to reset the eeprom, keep the VOLUME_UP button pressed during statup
  if (digitalRead(ENCODER_PUSH_BUTTON) == LOW)
  {
    EEPROM.write(eeprom_address, 0);
    EEPROM.commit();
    print(20, 20, &Orbitron_Light_24, "EEPROM RESETED");
    delay(3000);
  }

  // ICACHE_RAM_ATTR void rotaryEncoder(); see rotaryEncoder implementation below.
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), rotaryEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_B), rotaryEncoder, CHANGE);

  rx.setup(ESP32_I2C_SDA, ESP32_I2C_SCL);
  
  delay(300);



  // Checking the EEPROM content
  if (EEPROM.read(eeprom_address) == app_id)
  {
    readAllReceiverInformation();
  } else 
    rx.setVolume(volume);

  leds[0] = CRGB::Green;
  leds[1] = CRGB::Green;
  leds[2] = CRGB::Green;
  leds[3] = CRGB::Green;
  leds[4] = CRGB::Green;
  leds[5] = CRGB::Green;
  leds[6] = CRGB::Green;
  FastLED.show();

  // rx.setMaxDelaySetFrequency(20); // Sets the Max Delay after Set Frequency 

  useBand();
  showStatus();
}

// Turn the device and display on (true) or off (false)
void turnDisplay(bool v) {
  pinMode(PIN_LCD_BL, OUTPUT);
  digitalWrite(PIN_LCD_BL, v);   
  pinMode(PIN_POWER_ON, OUTPUT);
  digitalWrite(PIN_POWER_ON, v);
}

/**
 * Shows the first screen with some message.
 * Remove all code of this function if you do not wish it.
 */
void splash() {
  spr.fillSprite(TFT_BLACK);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.setFreeFont(&Orbitron_Light_24);
  spr.drawString(" PU2CLR BK1088", 140, 12);
  spr.drawString("Arduino Library", 140, 60);
  spr.pushSprite(0, 0);
  delay(700);
}

/**
 * Prints a given content on display 
 */
void print(uint8_t col, uint8_t lin, const GFXfont *font, const char *msg) {
  spr.setFreeFont(font);
  spr.drawString(msg, col, lin);
  spr.pushSprite(0, 0);
}

void printParam(const char *msg) {
  spr.fillRect(10, 120, 220, 40, TFT_BLACK);
  print(80,130,&Orbitron_Light_32, msg);
}

/*
   writes the conrrent receiver information into the eeprom.
   The EEPROM.update avoid write the same data in the same memory position. It will save unnecessary recording.
*/
void saveAllReceiverInformation()
{
  int addr_offset;

  EEPROM.begin(EEPROM_SIZE);
  EEPROM.write(eeprom_address, app_id);                 // stores the app id;
  EEPROM.write(eeprom_address + 1, rx.getVolume()); // stores the current Volume
  EEPROM.write(eeprom_address + 2, bandIdx);            // Stores the current band
  EEPROM.commit();

  addr_offset = 7;
  band[bandIdx].default_frequency = currentFrequency;

  for (int i = 0; i <= lastBand; i++)
  {
    EEPROM.write(addr_offset++, (band[i].default_frequency >> 8));   // stores the current Frequency HIGH byte for the band
    EEPROM.write(addr_offset++, (band[i].default_frequency & 0xFF)); // stores the current Frequency LOW byte for the band
    EEPROM.write(addr_offset++, band[i].step);       // Stores current step of the band
    EEPROM.commit();
  }
  EEPROM.end();
}

/**
 * reads the last receiver status from eeprom. 
 */
void readAllReceiverInformation()
{
  uint8_t volume;
  int addr_offset;
  int bwIdx;
  EEPROM.begin(EEPROM_SIZE);

  volume = EEPROM.read(eeprom_address + 1); // Gets the stored volume;
  bandIdx = EEPROM.read(eeprom_address + 2);

  addr_offset = 7;
  for (int i = 0; i <= lastBand; i++)
  {
    band[i].default_frequency = EEPROM.read(addr_offset++) << 8;
    band[i].default_frequency |= EEPROM.read(addr_offset++);
    band[i].step = EEPROM.read(addr_offset++);
  }
  EEPROM.end();
  currentFrequency = band[bandIdx].default_frequency;

  delay(50);
  rx.setVolume(volume);
}

/*
 * To store any change into the EEPROM, it is needed at least STORE_TIME  milliseconds of inactivity.
 */
void resetEepromDelay()
{
  elapsedCommand = storeTime = millis();
  itIsTimeToSave = true;
}

/**
    Set all command flags to false
    When all flags are disabled (false), the encoder controls the frequency
*/
void disableCommands()
{
  cmdBand = false;
  cmdVolume = false;
  cmdStep = false;
  cmdMenu = false;
  cmdSoftMuteMaxAtt = false;
  countClick = 0;
  showCommandStatus((char *) "VFO ");
}

/**
 * Reads encoder via interrupt
 * Use Rotary.h and  Rotary.cpp implementation to process encoder via interrupt
 * if you do not add ICACHE_RAM_ATTR declaration, the system will reboot during attachInterrupt call. 
 * With ICACHE_RAM_ATTR macro you put the function on the RAM.
 */
ICACHE_RAM_ATTR void  rotaryEncoder()
{ // rotary encoder events
  uint8_t encoderStatus = encoder.process();
  if (encoderStatus)
    encoderCount = (encoderStatus == DIR_CW) ? 1 : -1;
}

/**
 * Shows frequency information on Display
 */
void showFrequency()
{
  float freq, value;  
  int decimals; 

  // defines the frequency display format depending on the used band
  if ( band[bandIdx].mode == BK_MODE_FM )
  {
      freq = currentFrequency / 100.0;
      decimals = 1;
  }
  else
  {
    if ( currentFrequency  < 1000 ) { // LW or MW
      freq = (float) currentFrequency;
      decimals = 0;
    } else {
      freq = (float) currentFrequency / 1000.0;
      decimals = 3;
    }
  }

  value = freq * 10;

  // spr.setTextColor(TFT_YELLOW, TFT_YELLOW);   
  spr.fillSprite(TFT_BLACK); // clear screen
  spr.drawFloat(freq, decimals, 150, 64, 7); // shows the frequency

  // Builds the frequency ruler
  // spr.setTextColor(TFT_YELLOW, TFT_RED);   
  spr.fillTriangle(156, 104, 160, 114, 164, 104, TFT_RED); 
  spr.drawLine(160, 114, 160, 170, TFT_RED);
 
  int temp = value - 20;
  for (int i = 0; i < 40; i++)
  {
    if ((temp % 10) == 0)
    {
      spr.drawLine(i * 8, 170, i * 8, 140, color1);
      spr.drawLine((i * 8) + 1, 170, (i * 8) + 1, 140, color1);
      spr.drawFloat(temp / 10.0, 1, i * 8, 130, 2);
    }
    else if ((temp % 5) == 0 && (temp % 10) != 0)
    {
      spr.drawLine(i * 8, 170, i * 8, 150, color1);
      spr.drawLine((i * 8) + 1, 170, (i * 8) + 1, 150, color1);
    }
    else
    {
      spr.drawLine(i * 8, 170, i * 8, 160, color1);
    }
    temp = temp + 1;
  }

  showBattery();

  spr.pushSprite(0, 0);

  showRSSI();
  showMode();
}

/**
 * Shows the current mode
 */
void showMode() {
   spr.setTextColor(TFT_YELLOW, TFT_BLACK);   
   spr.setFreeFont(&FreeMono9pt7b);
   spr.drawString(band[bandIdx].name, 25, 35); 
   spr.setTextColor(TFT_WHITE, TFT_BLACK);  
   spr.pushSprite(0, 0);
}

void showBattery() {

  uint16_t colorBattery;
  int value = map(battery.getBatteryChargeLevel(true),0,100, 0,18);
  // int value = map(battery.getBatteryVolts() * 10,32,42,0,18);

  if (value < 5) 
     colorBattery = TFT_RED;
  else if (value < 11) 
     colorBattery = TFT_YELLOW;
  else 
     colorBattery = TFT_GREEN;      
  
  spr.drawRect(290, 6, 20, 9, TFT_WHITE);
  spr.fillRect(291, 7, 18, 7, TFT_BLACK);
  spr.fillRect(291, 7, value, 7, colorBattery);
  spr.fillRect(310, 8, 2, 5, TFT_WHITE);
  spr.pushSprite(0, 0);
  
}

/**
 * Shows some basic information on display
 */
void showStatus()
{
  showFrequency();
  showRSSI();


}

/*
 * Concert rssi to VU
*/
int getStrength(uint8_t rssi)
{
  if ((rssi >= 0) and (rssi <= 1))
    return 1; // S0
  if ((rssi > 1) and (rssi <= 1))
    return 2; // S1
  if ((rssi > 2) and (rssi <= 3))
    return  3; // S2
  if ((rssi > 3) and (rssi <= 4))
    return  4; // S3
  if ((rssi > 4) and (rssi <= 10))
    return  5; // S4
  if ((rssi > 10) and (rssi <= 16))
    return 6; // S5
  if ((rssi > 16) and (rssi <= 22))
    return 7; // S6
  if ((rssi > 22) and (rssi <= 28))
    return  8; // S7
  if ((rssi > 28) and (rssi <= 34))
    return 9; // S8
  if ((rssi > 34) and (rssi <= 44))
    return 10; // S9
  if ((rssi > 44) and (rssi <= 54))
    return 11; // S9 +10
  if ((rssi > 54) and (rssi <= 64))
    return 12; // S9 +20
  if ((rssi > 64) and (rssi <= 74))
    return 13; // S9 +30
  if ((rssi > 74) and (rssi <= 84))
    return 14; // S9 +40
  if ((rssi > 84) and (rssi <= 94))
    return 15; // S9 +50
  if (rssi > 94)
    return 16; // S9 +60
  if (rssi > 95)
    return 17; //>S9 +60

  return 0;
}

/**
 *   Shows the current RSSI and SNR status
 */
void showRSSI()
{
  spr.fillRect(240,20,76,80,TFT_BLACK); // Clear the indicator areas 
  
  for (int i = 0; i < getStrength(rssi); i++)
  {
    if (i < 9)
      spr.fillRect(244 + (i * 4), 80 - i, 2, 4 + i, TFT_GREEN);
    else
      spr.fillRect(244 + (i * 4), 80 - i, 2, 4 + i, TFT_RED);
  }

  if (band[bandIdx].mode == BK_MODE_FM)
  {
       spr.drawRoundRect(240, 20, 76, 22, 4, TFT_WHITE);
      if (rx.isStereo())
         spr.drawString("Stereo", 275, 31, 2);
      else
        spr.drawString("Mono", 275, 31, 2);
  }  
  spr.pushSprite(0, 0);
}



/**
 *   Shows the current step
 */
void showStep()
{

}


/*
 *  Shows the volume level on LCD
 */
void showVolume()
{
  char volAux[12];
  sprintf(volAux, "%2u", rx.getVolume());
  printParam(volAux);
}

/**
 * Show Soft Mute 
 */
void showSoftMute()
{

}

/**
 *   Sets Band up (1) or down (!1)
 */
void setBand(int8_t up_down)
{
  band[bandIdx].default_frequency = currentFrequency;
  if (up_down == 1)
    bandIdx = (bandIdx < lastBand) ? (bandIdx + 1) : 0;
  else
    bandIdx = (bandIdx > 0) ? (bandIdx - 1) : lastBand;
  useBand();
  delay(MIN_ELAPSED_TIME); // waits a little more for releasing the button.
  elapsedCommand = millis();
}

/**
 * Switch the radio to current band
 */
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
    rx.setAM(band[bandIdx].minimum_frequency, band[bandIdx].maximum_frequency, band[bandIdx].default_frequency, band[bandIdx].step, band[bandIdx].band_space);
  }
  delay(100);
  currentFrequency = band[bandIdx].default_frequency;
  rx.setFrequency(currentFrequency);
  delay(MIN_ELAPSED_TIME); // waits a little more for releasing the button.
  showStatus();
}


/**
 * Show cmd on display. It means you are setting up something.  
 */
void showCommandStatus(char * currentCmd )
{
  spr.setFreeFont(&Orbitron_Light_24);
  spr.fillRect(110, 0, 90, 25, TFT_BLACK);
  spr.drawString(currentCmd, 160, 12);
  spr.pushSprite(0, 0);
}

/**
 * Show menu options
 */
void showMenu() {
  spr.fillSprite(TFT_BLACK);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.setFreeFont(&Orbitron_Light_32);
  spr.drawString(menu[menuIdx], 100, 50);
  showCommandStatus( (char *) "Menu");
  spr.pushSprite(0, 0);
}

/**
  RDS Begin
 */


char *programInfo;
char *localTime;
long polling_rds = millis();

int progInfoIndex = 0;  // controls the part of the rdsMsg text will be shown on LCD 16x2 Display

long delayProgramInfo = millis();
long delayLocalTime = millis();


/**
  showProgramInfo - Shows the Program Information
*/
void showProgramInfo() {
  char txtAux[14];

  if (programInfo == NULL || strlen(programInfo) < 2 || (millis() - delayProgramInfo) < 1000L) return;
  delayProgramInfo = millis();
  programInfo[61] = '\0';  // Truncate the message to fit on display line
  strncpy(txtAux, &programInfo[progInfoIndex], 14);
  txtAux[13] = '\0';
  progInfoIndex += 3;
  if (progInfoIndex > 60) progInfoIndex = 0;

  spr.fillRect(0, 100, 152, 20, TFT_BLACK);
  spr.setTextColor(TFT_YELLOW, TFT_BLACK);  
  spr.setFreeFont(&FreeMono9pt7b);
  spr.drawString(txtAux,77,111);
  spr.pushSprite(0, 0);

}

void showTime() {
  if (localTime == NULL || strlen(localTime) < 4 || (millis() - delayLocalTime) < 50000) return;

  spr.fillRect(240, 100, 138, 20, TFT_BLACK);
  spr.setTextColor(TFT_YELLOW, TFT_BLACK);  
  spr.setFreeFont(&FreeMono9pt7b);
  spr.drawString(localTime,241,111);
  spr.pushSprite(0, 0);
  delayProgramInfo = millis(); // Stop showing Program Information for 10s
  delayLocalTime = millis(); 
} 


void clearRds() {
  localTime = programInfo = NULL;
  rx.clearRdsBuffer();
  progInfoIndex = 0;
  // clearLcdLine(0);
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

/**
  RDS END
*/   

/**
 * Switches the current step
 */
void doStep(int8_t v)
{

}


/**
 * Sets the audio volume
 */
void doVolume( int8_t v ) {
  if ( v == 1)
    rx.setVolumeUp();
  else
    rx.setVolumeDown();

  showVolume();
  delay(MIN_ELAPSED_TIME); // waits a little more for releasing the button.
}


/**
 *  Find a station. The direction is based on the last encoder move clockwise or counterclockwise
 */
void doSeek()
{
  rx.seek(BK_SEEK_WRAP, seekDirection, showFrequency);  // showFrequency will be called by the seek function during the process.
  delay(200);
  showStatus();
}

/**
 * Sets the Soft Mute Parameter
 */
void doSoftMute(int8_t v)
{

}

/**
 *  Menu options selection
 */
void doMenu( int8_t v) {
  menuIdx = (v == 1) ? menuIdx + 1 : menuIdx - 1;

  if (menuIdx > lastMenu)
    menuIdx = 0;
  else if (menuIdx < 0)
    menuIdx = lastMenu;

  showMenu();
  delay(MIN_ELAPSED_TIME); // waits a little more for releasing the button.
  elapsedCommand = millis();
}

/**
 * Starts the MENU action process
 */
void doCurrentMenuCmd() {
  disableCommands();
  switch (currentMenuCmd) {
     case 0:                 // VOLUME
      cmdVolume = true;
      showVolume();
      break;
    case 1:                 // STEP
      cmdStep = true;
      showStep();
      break;
    case 2: 
      cmdSoftMuteMaxAtt = true;
      showSoftMute();  
      break;
    case 3:
      doSeek();
      break;    
    default:
      showStatus();
      break;
  }
  currentMenuCmd = -1;
  elapsedCommand = millis();
}

/**
 * Return true if the current status is Menu command
 */
bool isMenuMode() {
  return (cmdMenu | cmdStep  | cmdVolume | cmdSoftMuteMaxAtt );
}

/**
 * Main loop
 */
void loop()
{
  // Check if the encoder has moved.
  if (encoderCount != 0)
  {
    if (cmdMenu)
      doMenu(encoderCount);
    else if (cmdStep)
      doStep(encoderCount);
    else if (cmdVolume)
      doVolume(encoderCount);
    else if (cmdSoftMuteMaxAtt)
      doSoftMute(encoderCount);
    else if (cmdBand)
      setBand(encoderCount);
    else
    {
      if (encoderCount == 1)
      {
        seekDirection = 1;
        rx.setFrequencyUp();
      }
      else
      {
        seekDirection = 0;
        rx.setFrequencyDown();
      }
      // Show the current frequency only if it has changed
      currentFrequency = rx.getFrequency();
      clearRds();
      showFrequency();
    }
    encoderCount = 0;
    resetEepromDelay();
  }
  else
  {
    if (digitalRead(ENCODER_PUSH_BUTTON) == LOW)
    {
      countClick++;
      if (cmdMenu)
      {
        currentMenuCmd = menuIdx;
        doCurrentMenuCmd();
      }
      else if (countClick == 1)
      { // If just one click, you can select the band by rotating the encoder
        if (isMenuMode())
        {
          disableCommands();
          showStatus();
          showCommandStatus((char *)"VFO ");
        }
        else
        {
          cmdBand = !cmdBand;
          showCommandStatus((char *)"Band");
        }
      }
      else
      { // GO to MENU if more than one click in less than 1/2 seconds.
        cmdMenu = !cmdMenu;
        if (cmdMenu)
          showMenu();
      }
      delay(MIN_ELAPSED_TIME);
      elapsedCommand = millis();
    }
  }

  // Show RSSI status only if this condition has changed
  if ((millis() - elapsedRSSI) > (MIN_ELAPSED_RSSI_TIME * 6))
  {
    int aux = rx.getRssi();
    if (rssi != aux && !isMenuMode())
    {
      rssi = aux;
      showRSSI();
    }
    elapsedRSSI = millis();
  }

  if ((millis() - elapsedClick) > ELAPSED_CLICK)
  {
    countClick = 0;
    elapsedClick = millis();
  }

  // Show the current frequency only if it has changed
  if (itIsTimeToSave)
  {
    if ((millis() - storeTime) > STORE_TIME)
    {
      saveAllReceiverInformation();
      storeTime = millis();
      itIsTimeToSave = false;
    }
  }

  if ((millis() - elapsedBattery) > ELAPSED_TIME_BATERRY )
  {
      elapsedBattery = millis();
      showBattery();
  }

  if ((millis() - polling_rds) > POLLING_RDS && band[bandIdx].mode == BK_MODE_FM)  {
    checkRDS();
    polling_rds = millis();
  }

}
