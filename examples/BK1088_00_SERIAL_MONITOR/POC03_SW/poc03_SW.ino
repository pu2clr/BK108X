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


uint16_t currentFrequency =7200;


long elapsedTime = millis();

uint16_t rssi, lastRssi;
uint16_t snr, lastSnr;

BK108X radio; 

void setup() {

  Serial.begin(9600);
  while(!Serial);

  radio.setup(SDA_PIN, CLK_PIN);
  radio.setVolume(20);

  radio.setAM(7200, 7450, currentFrequency, 5);
  showHelp();
  showStatus();

  lastRssi = rssi = radio.getRssi();
  lastSnr = snr = radio.getSnr();
}


void showHelp()
{
  Serial.print("\nCommands\n");
  Serial.println("==================================================");
  Serial.println("Type 1 to SW1; 2 to SW2; 3 to SW3; 4 to SW4 and 5 to SW5");
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

  freq =  currentFrequency / 1.0;

  Serial.print("\nYou are tuned on ");
  Serial.print(freq);
  Serial.print("KHz | RSSI: ");
  Serial.print(rssi);
  Serial.print("| SNR: ");
  Serial.print(snr);  
  Serial.print("| V - Volume: ");
  Serial.print(radio.getVolume());
  
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
    case '1':
      radio.setAM(7200, 7450, 7305, 5);
      break;
    case '2':
      radio.setAM(9300, 9800, 9600, 5);
      break;    
    case '3':
      radio.setAM(11600, 12000, 11940, 5);
      break;   
    case '4':
      radio.setAM(13500, 13900, 13600, 5);
      break;    
    case '5':
      radio.setAM(15100, 15800, 15300, 5);
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
      radio.seekHardware(0,1);
      break;
    case 's':
      radio.seekHardware(0,0);
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
    delay(100);
    showStatus();
  }
  if ( (millis() - elapsedTime) > 2000) {
    rssi = radio.getRssi(); 
    snr = radio.getSnr();
    
    if ( (lastRssi != rssi) || (lastSnr != snr) ) {
      showStatus();
      lastRssi = rssi; lastSnr = snr; 
    }
    
    elapsedTime = millis();
  }
}
