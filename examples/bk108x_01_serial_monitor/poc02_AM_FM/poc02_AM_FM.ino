/*
   Test and validation of the PU2CLR BK108X Arduino Library.
   It is a FM, MW (520 to 1710Khz) and SW 2.1 to 21.8MHz
   
   ATTENTION:  Please, avoid using the computer connected to the mains during testing.

   The main advantages of using this sketch are: 
    1) It is a easy way to check if your circuit is working;
    2) You do not need to connect any display device to make your radio works;
    3) You do not need connect any push buttons or encoders to change volume and frequency;
    4) The Arduino IDE is all you need to control the radio.  
   
   This sketch has been successfully tested on:
    1) Pro Mini 3.3V; 
    2) UNO (by using a voltage converter); 

  BK1088 and Arduino Pro Mini wire up
  
   | BK1088 pin   | Arduino pin | Description | 
   | -----------  | ----------- | ----------- |
   | CLK  (pin 7) |   A5        |             |
   | SDA  (pin 8) |   A4        |             |


  I strongly recommend starting with this sketch.

  See schematic: https://pu2clr.github.io/BK108X/

  PU2CLR BK108X API documentation: https://pu2clr.github.io/BK108X/extras/docs/html/index.html

  By Ricardo Lima Caratti, 2020.
*/

#include <BK108X.h>

#define SDA_PIN A4
#define CLK_PIN A5

uint16_t currentFM = 10390;
uint16_t currentAM = 620;
uint16_t currentSW = 7205; 

uint16_t currentFrequency;

BK108X radio; 

void setup() {

  Serial.begin(9600);
  while(!Serial);

  radio.setup(SDA_PIN, CLK_PIN);
  radio.setVolume(20);

  currentFrequency = currentFM;
  radio.setFM(8400, 10800, currentFrequency, 10);
  showHelp();
  showStatus();
}


void showHelp()
{
  Serial.print("\nCommands\n");
  Serial.println("==================================================");
  Serial.println("Type F to FM; A to MW; 1 to SW");
  Serial.println("Type U to increase and D to decrease the frequency");
  Serial.println("Type S or s to seek station Up or Down");
  Serial.println("Type + or - to volume Up or Down");
  Serial.println("Type 0 to show current status");
  Serial.println("Type ? to this help.");
  Serial.println("==================================================");
  delay(1000);
}

// Show current frequency
void showStatus()
{

  float freq;

  currentFrequency = radio.getFrequency();

  freq = (radio.getCurrentMode() == MODE_FM) ? currentFrequency / 100.0 : currentFrequency / 1.0;

  Serial.print("\nYou are tuned on ");
  Serial.print(freq);
  // Serial.print("MHz - RSSI: ");
  // Serial.print(radio.getRSSI());
  // Serial.print("V - Volume: ");
  // Serial.print(radio.getVolume());
}

void loop()
{

  if (Serial.available() > 0)
  {
    char key = Serial.read();
    switch (key)
    {
    case '+':
      radio.setVolumeUp();
      break;
    case '-':
      radio.setVolumeDown();
      break;
    case 'a':
    case 'A':
      currentFM = currentFrequency;
      radio.setAM(550, 1710, currentAM, 10);
      break;
    case 'f':
    case 'F':
      currentAM = currentFrequency;
      radio.setFM(8700, 10800, currentFM, 100);
      break;
    case '1':
      radio.setFM(7200, 7500, currentSW, 5);
      break;
    case 'U':
    case 'u':
      radio.setFrequencyUp();
      break;
    case 'D':
    case 'd':
      radio.setFrequencyDown();
      break;
    case 'S':
      radio.seek(0,1,showStatus);
      break;
    case 's':
      radio.seek(0,0,showStatus);
      break;
    case '0':
      showStatus();
      break;
    case '?':
      showHelp();
      break;
    default:
      break;
    }
    showStatus();
  }
}
