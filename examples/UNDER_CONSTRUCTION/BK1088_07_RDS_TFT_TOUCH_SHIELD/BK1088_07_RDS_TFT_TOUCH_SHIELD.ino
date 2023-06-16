/*
  This sketch uses the mcufriend TFT touch Display Shield.

  Driver LCD: ST7781
  Controller: ILI9341 (ili9481 ili9468, ili9488 hx8357, or r61581)
   Resolution: 240 x 320



  You can use it on Mega2560 or DUE.
  It is a RDS example.

  The  purpose  of  this  example  is  to  demonstrate a prototype  receiver based  on  the  BK1088-D60 or Si4732-A10  and  the
  "PU2CLR BK1088 Arduino Library". It is not the purpose of this prototype  to provide you a beautiful interface. You can do it better.


  It is important to know the SSB support works on BK1088-D60 and SI4732-A10 devices.

  Features:
  1) This sketch has been successfully tested on Arduino Mega2560 and Arduino DUE;
  2) It uses the touch screen interface provided by mcufriend TFT;
  3) Encoder;
  4) FM, AM (MW and SW) and SSB (LSB and USB);
  5) Audio bandwidth filter 0.5, 1, 1.2, 2.2, 3 and 4kHz;
  6) BFO Control;
  7) RDS;
  8) Frequency step switch (1, 5 and 10kHz).

  Wire up

  Function                MEGA2560 or DUE  Pin
  ----------------------  --------------------
  SDA                     20
  SCL                     21
  ENCODER_A               18        - On Arduino DUE all Digital pin can be used with Interrupt
  ENCODER_B               19        - On Arduino DUE all Digital pin can be used with Interrupt
  ENCODER PUSH BUTTON     23

  ATTENTION: Your toutch screen needs to be calibrated to work properly.
             To do that, use the TouchScreen_Calibr_native.ino that comes with MCUFRIEND_kbv library.
             Read the TouchScreen_Calibr_native.ino and check the XP, XM , YP and YM pins configuration.
             You might need to change the XP, XM , YP and YM values in the TouchScreen_Calibr_native.ino
             depending on the display you are using.

  Libraries used: BK108X; Adafruit_GFX; MCUFRIEND_kbv; FreeDefaultFonts; TouchScreen;

  Prototype documentation: https://pu2clr.github.io/BK1088/
  PU2CLR Si47XX API documentation: https://pu2clr.github.io/BK1088/extras/apidoc/html/

  By Ricardo Lima Caratti, Feb 2020
*/

#include <BK108X.h>
#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>

#include "Rotary.h"

#include <patch_init.h>  // SSB patch for whole SSBRX initialization string

#define MINPRESSURE 200
#define MAXPRESSURE 1000

#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

#define ENCODER_PUSH_BUTTON 23  // Used to switch BFO and VFO or other function

// Enconder PINs (interrupt pins used on DUE. All Digital DUE Pins can be used as interrupt)
#define ENCODER_PIN_A 18
#define ENCODER_PIN_B 19

#define I2C_SDIO 20
#define I2C_SCLK 21


// Encoder control variables
volatile int encoderCount = 0;

uint16_t currentFrequency;
uint16_t previousFrequency;

char buffer[255];
char bufferFreq[10];
char bufferStereo[10];

Rotary encoder = Rotary(ENCODER_PIN_A, ENCODER_PIN_B);
MCUFRIEND_kbv tft;
BK108X rx;

// ALL Touch panels and wiring is DIFFERENT
// copy-paste results from TouchScreen_Calibr_native.ino
// ATTENTION: if you do not get success at first, check and change the XP, XM , YP and YM pins.
// Read TouchScreen_Calibr_native.ino  (MCUFRIEND shield shares pins with the TFT).

// TFT Touch shield  (my old and original MFUFIEND toutch screen)
// const int XP = 6, XM = A2, YP = A1, YM = 7; //240x320 ID=0x9328
// const int TS_LEFT = 170, TS_RT = 827, TS_TOP = 130, TS_BOT = 868;

// TFT Touch shield 2 (my new kind of mcufriend toutch screen)
const int XP = 7, XM = A1, YP = A2, YM = 6;  //240x320 ID=0x2053
const int TS_LEFT = 155, TS_RT = 831, TS_TOP = 158, TS_BOT = 892;

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 320);
Adafruit_GFX_Button bNextBand, bPreviousBand, bVolumeUp, bVolumeDown, bSeekUp, bSeekDown, bStep, bAudioMute, bAM, bLSB, bUSB, bFM, bMW, bSW, bFilter, bAGC;

int pixel_x, pixel_y;  //Touch_getXY() updates global vars
bool Touch_getXY(void) {
  TSPoint p = ts.getPoint();
  pinMode(YP, OUTPUT);  //restore shared pins
  pinMode(XM, OUTPUT);
  digitalWrite(YP, HIGH);  //because TFT control pins
  digitalWrite(XM, HIGH);
  bool pressed = (p.z > MINPRESSURE && p.z < MAXPRESSURE);
  if (pressed) {
    pixel_x = map(p.x, TS_LEFT, TS_RT, 0, tft.width());  //.kbv makes sense to me
    pixel_y = map(p.y, TS_TOP, TS_BOT, 0, tft.height());
  }
  return pressed;
}

void setup(void) {

  // Encoder pins
  pinMode(ENCODER_PIN_A, INPUT_PULLUP);
  pinMode(ENCODER_PIN_B, INPUT_PULLUP);
  pinMode(ENCODER_PUSH_BUTTON, INPUT_PULLUP);

  uint16_t ID = tft.readID();

  if (ID == 0xD3D3)
    ID = 0x9486;  // write-only shield
  tft.begin(ID);
  tft.setRotation(0);  //PORTRAIT
  tft.fillScreen(BLACK);

  // tft.setFont(&FreeSans12pt7b);
  showText(55, 30, 2, NULL, GREEN, "BK108X");
  showText(55, 90, 2, NULL, YELLOW, "Arduino");
  showText(55, 160, 2, NULL, YELLOW, "Library");
  showText(55, 240, 2, NULL, WHITE, "By PU2CLR");
  delay(3000);

  tft.fillScreen(BLACK);

  showTemplate();

  // Atach Encoder pins interrupt
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), rotaryEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_B), rotaryEncoder, CHANGE);

  rx.setup(I2C_SDIO, I2C_SCLK);

  // Set up the radio for the current band (see index table variable bandIdx )
  delay(100);
  useBand();
  currentFrequency = previousFrequency = rx.getFrequency();
  rx.setVolume(25);
  tft.setFont(NULL);  // default font
}

/*
   Use Rotary.h and  Rotary.cpp implementation to process encoder via interrupt

*/
void rotaryEncoder() {  // rotary encoder events
  uint8_t encoderStatus = encoder.process();
  if (encoderStatus) {
    if (encoderStatus == DIR_CW) {
      encoderCount = 1;
    } else {
      encoderCount = -1;
    }
  }
}

/*
   Shows a text on a given position; with a given size and font, and with a given color

   @param int x column
   @param int y line
   @param int sz font size
   @param const GFXfont *f font type
   @param uint16_t color
   @param char * msg message
*/
void showText(int x, int y, int sz, const GFXfont *f, uint16_t color, const char *msg) {
  tft.setFont(f);
  tft.setCursor(x, y);
  tft.setTextColor(color);
  tft.setTextSize(sz);
  tft.print(msg);
}

/*
    Prevents blinking during the frequency display.
    Erases the old char/digit value if it has changed and print the new one.
*/
void printText(int col, int line, int sizeText, char *oldValue, char *newValue, uint16_t color, uint8_t space) {
  int c = col;
  char *pOld;
  char *pNew;

  pOld = oldValue;
  pNew = newValue;

  tft.setTextSize(sizeText);

  // prints just changed digits
  while (*pOld && *pNew) {
    if (*pOld != *pNew) {
      tft.setTextColor(BLACK);
      tft.setCursor(c, line);
      tft.print(*pOld);
      tft.setTextColor(color);
      tft.setCursor(c, line);
      tft.print(*pNew);
    }
    pOld++;
    pNew++;
    c += space;
  }

  // Is there anything else to erase?
  tft.setTextColor(BLACK);
  while (*pOld) {
    tft.setCursor(c, line);
    tft.print(*pOld);
    pOld++;
    c += space;
  }

  // Is there anything else to print?
  tft.setTextColor(color);
  while (*pNew) {
    tft.setCursor(c, line);
    tft.print(*pNew);
    pNew++;
    c += space;
  }

  // Save the current content to be tested next time
  strcpy(oldValue, newValue);
}

void showTemplate() {

  // Área reservada à frequência
  tft.drawRect(0, 0, tft.width(), 50, WHITE);

  tft.drawRect(0, 100, tft.width(), 160, CYAN);
  tft.setFont(NULL);
  bPreviousBand.initButton(&tft, 30, 120, 40, 30, WHITE, CYAN, BLACK, (char *)"Band-", 1);
  bNextBand.initButton(&tft, 90, 120, 40, 30, WHITE, CYAN, BLACK, (char *)"Band+", 1);
  bVolumeDown.initButton(&tft, 150, 120, 40, 30, WHITE, CYAN, BLACK, (char *)"Vol-", 1);
  bVolumeUp.initButton(&tft, 210, 120, 40, 30, WHITE, CYAN, BLACK, (char *)"Vol+", 1);
  bSeekDown.initButton(&tft, 30, 160, 40, 30, WHITE, CYAN, BLACK, (char *)"Seek-", 1);
  bSeekUp.initButton(&tft, 90, 160, 40, 30, WHITE, CYAN, BLACK, (char *)"Seek+", 1);
  bAudioMute.initButton(&tft, 150, 160, 40, 30, WHITE, CYAN, BLACK, (char *)"Mute", 1);
  bStep.initButton(&tft, 210, 160, 40, 30, WHITE, CYAN, BLACK, (char *)"Step", 1);
  bFM.initButton(&tft, 30, 200, 40, 30, WHITE, CYAN, BLACK, (char *)"FM", 1);
  bMW.initButton(&tft, 90, 200, 40, 30, WHITE, CYAN, BLACK, (char *)"MW", 1);
  bSW.initButton(&tft, 150, 200, 40, 30, WHITE, CYAN, BLACK, (char *)"SW", 1);
  bAGC.initButton(&tft, 210, 200, 40, 30, WHITE, CYAN, BLACK, (char *)"ATT", 1);
  bAM.initButton(&tft, 30, 240, 40, 30, WHITE, CYAN, BLACK, (char *)"AM", 1);
  bLSB.initButton(&tft, 90, 240, 40, 30, WHITE, CYAN, BLACK, (char *)"LSB", 1);
  bUSB.initButton(&tft, 150, 240, 40, 30, WHITE, CYAN, BLACK, (char *)"USB", 1);
  bFilter.initButton(&tft, 210, 240, 40, 30, WHITE, CYAN, BLACK, (char *)"|Y|", 1);

  // Exibe os botões (teclado touch)
  bNextBand.drawButton(true);
  bPreviousBand.drawButton(true);
  bVolumeUp.drawButton(true);
  bVolumeDown.drawButton(true);
  bSeekUp.drawButton(true);
  bSeekDown.drawButton(true);
  bStep.drawButton(true);
  bAudioMute.drawButton(true);
  bFM.drawButton(true);
  bMW.drawButton(true);
  bSW.drawButton(true);
  bAM.drawButton(true);
  bLSB.drawButton(true);
  bUSB.drawButton(true);
  bFilter.drawButton(true);
  bAGC.drawButton(true);

  showText(0, 270, 1, NULL, YELLOW, "PU2CLR BK108X Arduino Library-Example");
  showText(0, 285, 1, NULL, YELLOW, "DIY - You can make it better.");

  showText(0, 302, 1, NULL, GREEN, "RSSI");
  tft.drawRect(30, 298, 210, 12, CYAN);

  tft.setFont(NULL);
}

/*
    Prevents blinking during the frequency display.
    Erases the old digits if it has changed and print the new digit values.
*/
void showFrequencyValue(int col, int line, char *oldValue, char *newValue, uint16_t color) {

  int c = col;

  // prints just changed digits
  while (*oldValue && *newValue) {
    if (*oldValue != *newValue) {
      tft.drawChar(c, line, *oldValue, BLACK, BLACK, 4);
      tft.drawChar(c, line, *newValue, color, BLACK, 4);
    }
    oldValue++;
    newValue++;
    c += 25;
  }

  // Is there anything else to erase?
  while (*oldValue) {
    tft.drawChar(c, line, *oldValue, BLACK, BLACK, 4);
    oldValue++;
    c += 25;
  }

  // Is there anything else to print?
  while (*newValue) {
    tft.drawChar(c, line, *newValue, color, BLACK, 4);
    newValue++;
    c += 25;
  }
}

void showFrequency() {
  // TO DO

  // showFrequencyValue(10, 10, bufferFreq, buffer, color);
  // tft.setFont(NULL);  // default font
  // strcpy(bufferFreq, buffer);
}

// Will be used by seekStationProgress
void showFrequencySeek(uint16_t freq) {
  previousFrequency = currentFrequency = freq;
  showFrequency();
}

void showStatus() {
}

void showRSSI() {
}



/**
 *. RDS - Begin
 */
char *programInfo;
char *stationName;
char *localTime;
char bufferStatioName[15];
char bufferProgramInfo[65];
char bufferlocalTime[32];

void showProgramInfo() {
}

void showStationName() {
}

void showLocalTime() {
}

void checkRDS() {
}

/**
 * RDS - END
 */


void showVolume() {
}

/*
   Goes to the next band (see Band table)
*/
void bandUp() {
}

/*
   Goes to the previous band (see Band table)
*/
void bandDown() {
}



/*
   Switch the radio to current band
*/
void useBand() {
}


void switchStep() {
}



/* two buttons are quite simple
*/
void loop(void) {
  bool down = Touch_getXY();
  bNextBand.press(down && bNextBand.contains(pixel_x, pixel_y));
  bPreviousBand.press(down && bPreviousBand.contains(pixel_x, pixel_y));
  bVolumeUp.press(down && bVolumeUp.contains(pixel_x, pixel_y));
  bVolumeDown.press(down && bVolumeDown.contains(pixel_x, pixel_y));
  bSeekUp.press(down && bSeekUp.contains(pixel_x, pixel_y));
  bSeekDown.press(down && bSeekDown.contains(pixel_x, pixel_y));
  bStep.press(down && bStep.contains(pixel_x, pixel_y));
  bAudioMute.press(down && bAudioMute.contains(pixel_x, pixel_y));
  bFM.press(down && bFM.contains(pixel_x, pixel_y));
  bMW.press(down && bMW.contains(pixel_x, pixel_y));
  bSW.press(down && bSW.contains(pixel_x, pixel_y));
  bAM.press(down && bAM.contains(pixel_x, pixel_y));
  bLSB.press(down && bLSB.contains(pixel_x, pixel_y));
  bUSB.press(down && bUSB.contains(pixel_x, pixel_y));
  bFilter.press(down && bFilter.contains(pixel_x, pixel_y));
  bAGC.press(down && bAGC.contains(pixel_x, pixel_y));

  // Check if the encoder has moved.
  if (encoderCount != 0) {
    if (encoderCount == 1)
      rx.setFrequencyUp();
    else
      rx.setFrequencyDown();

    // Show the current frequency only if it has changed
    currentFrequency = rx.getFrequency();
    showFrequency();
    encoderCount = 0;
  }


  if (bNextBand.justPressed())
    bandUp();

  if (bPreviousBand.justPressed())
    bandDown();

  // Volume
  if (bVolumeUp.justPressed()) {

    rx.setVolumeUp();

  }

  if (bVolumeDown.justPressed()) {

    rx.setVolumeDown();

  }

  // SEEK Test
  if (bSeekUp.justPressed()) {
    rx.seek(showFrequencySeek, BK_SEEK_UP);
    // rx.seekNextStation(); // This method does not show the progress
    currentFrequency = rx.getFrequency();
    showStatus();
  }

  if (bSeekDown.justPressed()) {
    rx.seek(showFrequencySeek, BK_SEEK_DOWN);
    // rx.seekPreviousStation(); // This method does not show the progress
    currentFrequency = rx.getFrequency();
    showStatus();
  }

  // Mute
  if (bAudioMute.justPressed()) {
    rx.setAudioMute(true);
  }

  if (bAM.justPressed()) {
    useBand();

  }

  if (bFM.justPressed()) {
    useBand();

  }

  if (bMW.justPressed()) {
    useBand();
  }

  if (bSW.justPressed()) {
    useBand();
  }


  if (bStep.justPressed()) {
    switchStep();
  }

  if (digitalRead(ENCODER_PUSH_BUTTON) == LOW) {
    showStatus();
  }

  /*

  if ( FM ) {
    if (currentFrequency != previousFrequency) {
      tft.fillRect(54, 59, 250, 36, BLACK);
      rx.clearRdsBuffer();
      previousFrequency = currentFrequency;
    }
    checkRDS();
  }
  */

  delay(5);
}
