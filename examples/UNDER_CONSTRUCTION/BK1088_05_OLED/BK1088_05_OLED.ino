/**
 * It is FM and AM (MW and SW) BK1088 based receiver controlled by a Arduino Pro Mini. You can also use 
 * other Arduino boards. However, see documentation before try that: https://github.com/pu2clr/BK108X 
 * 
 * This receiver use a OLED 92" (128 x 64), 6 push buttons and an Encoder. 
 *  
 * This sketch uses the following libraries:
 *  PU2CLR AKC Arduino Library;
 *  Adafruit_GFX; and 
 *  Adafruit_SSD1306
 *  
 * This software can be freely distributed using the MIT Free Software model.
 * 
 * BK1088 and Arduino Pro Mini wire up
 * 
 * | BK1088 pin | Arduino pin | Description | 
 * | ----------- | ----------- | ----------- |
 * |    5        |   12        | You can use the Arduino RST pin. In this case set RESET_PIN to -1 |
 * |    6 (SCL)  |   A5 SCL    | I2C bus clock | 
 * |    7 (SDA)  |   A4 SDA    | I2C bus data  | 
 * 
 *    
 * @see schematic on https://github.com/pu2clr/AKC695X#schematic
 * 
 * @author Ricardo Lima Caratti (pu2clr@gmail.com)
 * @copyright Copyright (c) 2020
 */

#include <BK108X.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "Rotary.h"

#define RESET_PIN 12 // You can use the Arduino RST pin. In this case, set it to -1

#define OLED_I2C_ADDRESS 0x3C   // Check the I2C bus of your OLED device
#define SCREEN_WIDTH 128        // OLED display width, in pixels
#define SCREEN_HEIGHT 64        // OLED display height, in pixels
#define OLED_RESET -1           // Reset pin # (or -1 if sharing Arduino reset pin)

#define ENCODER_PIN_A 2
#define ENCODER_PIN_B 3

// Buttons controllers
#define BT_BAND_MODE_SWITCH_UP 4     // Switch to: FM -> MW(AM) -> SW1(AM) -> SW2(AM) etc
#define BT_BAND_MODE_SWITCH_DOWN 5   //
#define BT_VOL_UP    6               // Volume Up
#define BT_VOL_DOWN  7               // Volume Down
#define BT_SEEK_UP   8               // Seek Station Up
#define BT_SEEK_DOWN 9               // Seek Station Down

#define MIN_ELAPSED_TIME 100

long elapsedButton = millis();

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


const int lastBand = (sizeof band / sizeof(band)) - 1;
int bandIdx = 0; // FM

uint8_t seekDirection = 1;  // 0 = Down; 1 = Up. This value is set by the last encoder direction.


// The array sizes below can be optimized. 
char oldFreq[20];
char oldMode[20];
char oldUnit[20];
char oldStep[20];
char oldRssi[20];
char oldVolume[20];
char oldVbat[20];
char oldStereo[20];

Rotary encoder = Rotary(ENCODER_PIN_A, ENCODER_PIN_B);
// Encoder control variables
volatile int encoderCount = 0;

Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

BK108X rx;

uint16_t currentFM = 103;
uint16_t currentAM = 810;

uint16_t currentFrequency;

void setup()
{

  pinMode(ENCODER_PIN_A, INPUT_PULLUP);
  pinMode(ENCODER_PIN_B, INPUT_PULLUP);

  pinMode(BT_BAND_MODE_SWITCH_UP, INPUT_PULLUP);
  pinMode(BT_BAND_MODE_SWITCH_DOWN, INPUT_PULLUP);
  pinMode(BT_VOL_UP, INPUT_PULLUP);
  pinMode(BT_VOL_DOWN, INPUT_PULLUP);
  pinMode(BT_SEEK_UP, INPUT_PULLUP);
  pinMode(BT_SEEK_DOWN, INPUT_PULLUP);  

  // Encoder interrupt
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), rotaryEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_B), rotaryEncoder, CHANGE);


  oled.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS);
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);

  splash();

  // Start the receiver using ATMega328 I2C pins (4 = SDA/SDIO, 5 = CLK/SCLK)
  rx.setup(4,5);

  rx.setFM(band[bandIdx].minimum_frequency, band[bandIdx].maximum_frequency, band[bandIdx].default_frequency, band[bandIdx].step);
  
  showStatus();
}

void rotaryEncoder()
{ // rotary encoder events
  uint8_t encoderStatus = encoder.process();
  if (encoderStatus)
  {
    if (encoderStatus == DIR_CW)
    {
      encoderCount = 1;
    }
    else
    {
      encoderCount = -1;
    }
  }
}

/**
 * Clear a given array char string
 */
inline void clearBuffer(char *p)
{
  p[0] = '\0';
}

/**
 * Chear all string buffer information
 * These strings are used to avoid blinking on display.
 * See printValue function.
 */
void resetBuffer()
{
  clearBuffer(oldFreq);
  clearBuffer(oldMode);
  clearBuffer(oldUnit);
  clearBuffer(oldStep);
  clearBuffer(oldRssi);
  clearBuffer(oldVolume);
  clearBuffer(oldVbat);
  clearBuffer(oldStereo);
}

/**
 * Splash - Change it for your introduction text.
 */
void splash()
{
  oled.setTextSize(1); // Draw 2X-scale text
  oled.setCursor(40, 0);
  oled.print("AKC695X");
  oled.setCursor(20, 10);
  oled.print("Arduino Library");
  oled.display();
  delay(500);
  oled.setCursor(30, 35);
  oled.print("OLED-EXAMPLE");
  oled.setCursor(10, 50);
  oled.print("V1.0.0 - By PU2CLR");
  oled.display();
  delay(2000);
}

/*
    Writes just the changed information on Display
    Prevents blinking on display and also noise.
    Erases the old digits if it has changed and print the new digit values.
*/
void printValue(int col, int line, char *oldValue, char *newValue, int space, int textSize)
{
  int c = col;
  char *pOld;
  char *pNew;

  pOld = oldValue;
  pNew = newValue;

  oled.setTextSize(textSize);

  // prints just changed digits
  while (*pOld && *pNew)
  {
    if (*pOld != *pNew)
    {
      oled.setTextColor(SSD1306_BLACK);
      oled.setCursor(c, line);
      oled.write(*pOld);
      oled.setTextColor(SSD1306_WHITE);
      oled.setCursor(c, line);
      oled.write(*pNew);
    }
    pOld++;
    pNew++;
    c += space;
  }
  // Is there anything else to erase?
  oled.setTextColor(SSD1306_BLACK);
  while (*pOld)
  {
    oled.setCursor(c, line);
    oled.write(*pOld);
    pOld++;
    c += space;
  }
  // Is there anything else to print?
  oled.setTextColor(SSD1306_WHITE);
  while (*pNew)
  {
    oled.setCursor(c, line);
    oled.write(*pNew);
    pNew++;
    c += space;
  }

  // Save the current content to be tested next time
  strcpy(oldValue, newValue);
}

/**
 * Show the current frequency
 * 
 */
void showFrequency()
{
  char freq[15];
  char tmp[15];
  char *unit;
  char *bandMode;
  char *stereo;
  

  currentFrequency = rx.getFrequency();

  sprintf(tmp, "%5u", currentFrequency);

  if (band[bandIdx].mode == 1) // FM
  {
    freq[0] = tmp[0];
    freq[1] = tmp[1];
    freq[2] = tmp[2];
    freq[3] = tmp[3];
    freq[4] = '.';
    freq[5] = tmp[4];    
    freq[6] = '\0';
    unit = (char *) "MHz";
    bandMode =  (char *) "FM";
    stereo = (rx.isStereo())? (char *) "Stereo": (char *)"Mono";
  }
  else // AM
  {
    freq[0] = ' ';
    freq[1] = tmp[0];
    freq[2] = tmp[1];
    freq[3] = tmp[2];
    freq[4] = tmp[3];
    freq[5] = tmp[4];
    freq[6] = '\0';
    unit = (char *) "kHz";
    bandMode = (char *) "AM";
    stereo = (char *) "       ";
  }

  printValue(23, 0, oldFreq, freq, 12, 2);

  printValue(0, 20, oldStereo, stereo, 6, 1);

  printValue(0, 0, oldMode, bandMode, 7, 1);
  printValue(105, 0, oldUnit, unit, 7, 1);
  showRSSI();

  oled.display();
}

// Show current frequency
void showStatus()
{

  oled.clearDisplay();
  resetBuffer();

  oled.drawLine(0, 17, 130, 17, SSD1306_WHITE);
  oled.drawLine(0, 52, 130, 52, SSD1306_WHITE);

  showFrequency();

  showVolume();
  showVbat();

  oled.display();
}

/* *******************************
   Shows RSSI status
*/
void showRSSI()
{

  char sR[20];

  sprintf(sR,"RSSI:%3.3idBuV", rx.getRssi());
  printValue(0, 40, oldRssi, sR, 6, 1);
  oled.display();

}

/*
   Shows the volume level on LCD
*/
void showVolume()
{
  char sVolume[20];
  sprintf(sVolume, "Vol: %2.2u", rx.getVolume());
  printValue(80, 56, oldVolume, sVolume, 6, 1);
  oled.display();
}

/*
 * Shows the battery voltage 
 */
void showVbat()
{
  /*
  char sV[20];
  float v = rx.getSupplyVoltage();
  dtostrf(v, 2, 1, sV); // This may not work on some architectures (for example: Arduino DUE). 
  strcat(sV,"V");
  printValue(0, 56, oldVbat, sV, 6, 1);
  oled.display();
  */
 
}

/*********************************************************/

/*
 * Button - Volume control
 */
void volumeButton(byte d)
{
  
  if (d == 1)
    rx.setVolumeUp();
  else
    rx.setVolumeDown();

  showVolume();
  delay(MIN_ELAPSED_TIME); // waits a little more for releasing the button.
}


void seekButton( uint8_t up_down ) {
  rx.seek(BK_SEEK_WRAP, seekDirection, showFrequency);  // showFrequency will be called by the seek function during the process.
  delay(200);
  showStatus();
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
    rx.setAM(band[bandIdx].minimum_frequency, band[bandIdx].maximum_frequency, band[bandIdx].default_frequency, band[bandIdx].step, band[bandIdx].band_space);
  }
  delay(100);
  currentFrequency = band[bandIdx].default_frequency;
  rx.setFrequency(currentFrequency);
  delay(MIN_ELAPSED_TIME); // waits a little more for releasing the button.
  showStatus();
}


void loop()
{
  // Check if the encoder has moved.
  if (encoderCount != 0)
  {
    if (encoderCount == 1) {
      rx.setFrequencyUp();
      seekDirection = 1;
    }  
    else {
      rx.setFrequencyDown();
      seekDirection = 0;
    }

    showFrequency();
    encoderCount = 0;
  }

  // Check button commands
  if ((millis() - elapsedButton) > MIN_ELAPSED_TIME)
  {
    // check if some button is pressed
    if (digitalRead(BT_BAND_MODE_SWITCH_UP) == LOW)
       bandUp(); 
    else if (digitalRead(BT_BAND_MODE_SWITCH_DOWN) == LOW)
      bandDown();
    else if (digitalRead(BT_VOL_UP) == LOW)
      volumeButton(1);
    else if (digitalRead(BT_VOL_DOWN) == LOW)
      volumeButton(-1);
    else if  (digitalRead(BT_SEEK_UP) == LOW)
      seekButton(1);
    else if  (digitalRead(BT_SEEK_DOWN) == LOW)
      seekButton(0);
  }  

  delay(10);
}
