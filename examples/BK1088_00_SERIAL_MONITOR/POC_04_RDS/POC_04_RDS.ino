/*
   Test and validation of FM RDS .
      
   ATTENTION:  Please, avoid using the computer connected to the mains during testing.

   This sketch has been successfully tested on:
    1) Pro Mini 3.3V; 
    2) UNO (by using a voltage converter); 

  BK1088 and Arduino Pro Mini wire up
  
   | BK1088 pin   | Arduino pin | Description | 
   | -----------  | ----------- | ----------- |
   | CLK  (pin 7) |   A5        |             |
   | SDA  (pin 8) |   A4        |             |

  BK1088 and ESP32 DEV Modeule
  
   | BK1088 pin   | ESP32 DEV Modele | 
   | -----------  | ---------------- | 
   | CLK  (pin 7) |  GPIO22          |
   | SDA  (pin 8) |  GPIO21          |

  See schematic: https://pu2clr.github.io/BK108X/

  PU2CLR BK108X API documentation: https://pu2clr.github.io/BK108X/extras/docs/html/index.html

  By Ricardo Lima Caratti, 2023.
*/

#include <BK108X.h>



#define SDA_PIN A4
#define CLK_PIN A5

// #define SDA_PIN 21
// #define CLK_PIN 22


uint16_t currentFrequency;


long elapsedTime = millis();
long delayRDSInfo = millis();

uint16_t rssi, lastRssi;
uint16_t snr, lastSnr;
uint8_t stereo, lastStereo;

BK108X rx;

void setup() {

  Serial.begin(9600);

  rx.setup(SDA_PIN, CLK_PIN);
  currentFrequency = 8990;  // Using a strong station that broadcast RDS service
  rx.setFM(8400, 10800, currentFrequency, 10);
  rx.setVolume(20);
  rx.setRds(true);
  showHelp();
  showStatus();

  lastRssi = rssi = rx.getRssi();
  lastSnr = snr = rx.getSnr();
  lastStereo = stereo = rx.isStereo();
}


void showHelp() {
  Serial.print("\nCommands\n");
  Serial.println("==================================================");
  Serial.println("Type U to increase and D to decrease the frequency");
  Serial.println("Type S or s to seek station Up or Down");
  Serial.println("Type + or - to volume Up or Down");
  Serial.println("Type 0 to show current status");
  Serial.println("Type ? to this help.");
  Serial.println("==================================================");
  delay(2000);
}

// Show current frequency
void showStatus() {

  char *unt;

  currentFrequency = rx.getFrequency();

  unt = (char *)"MHz";

  Serial.print("\nYou are tuned on ");
  Serial.print(rx.formatCurrentFrequency());
  Serial.print(unt);
  Serial.print(" | RSSI: ");
  Serial.print(rssi);
  Serial.print("| SNR: ");
  Serial.print(snr);
  Serial.print("| V - Volume: ");
  Serial.print(rx.getVolume());
  Serial.print(" | Stereo: ");
  Serial.print(stereo);
  // Serial.print(" | RDS: ");
  // Serial.print(rx.getRdsReady());
}

void processRdsInfo() {

  char *pProgramInfo;
  char *pStationName;
  char *pStarionInfo;
  char *pLocalTime;

  if ((millis() - delayRDSInfo) > 50) {
    Serial.print("\nGroupType....: ");
    Serial.print(rx.getRdsGroupType());
    Serial.print("\nVersion Code.: ");
    Serial.print(rx.getRdsVersionCode());
    Serial.print("\nProgramType..: ");
    Serial.print(rx.getRdsProgramType());
    pStationName = rx.getRdsStationName();
    if (pStationName != NULL) {
      Serial.print("\nStationName..: ");
      Serial.print(pStationName);
    }
    pProgramInfo = rx.getRdsProgramInformation();
    if (pProgramInfo != NULL) {
      Serial.print("\nProgramInfo..: ");
      Serial.print(pProgramInfo);
    }

    pStarionInfo = rx.getRdsStationInformation();
    if (pStarionInfo != NULL) {
      Serial.print("\nStationInfo..: ");
      Serial.print(pStarionInfo);
    }

    pLocalTime = rx.getRdsLocalTime();
    if (pLocalTime != NULL) {
      Serial.print("\nLocalTime..: ");
      Serial.print(pLocalTime);
    }


    delayRDSInfo = millis();
  }
}

void loop() {

  if (Serial.available() > 0) {
    char key = Serial.read();
    rx.clearRdsBuffer();
    switch (key) {
      case '+':
        rx.setVolumeUp();
        break;
      case '-':
        rx.setVolumeDown();
        break;
      case 'U':
      case 'u':
        rx.setFrequencyUp();
        break;
      case 'D':
      case 'd':
        rx.setFrequencyDown();
        break;
      case 'S':
        rx.seekHardware(0, 1);
        break;
      case 's':
        rx.seekHardware(0, 0);
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
  if ((millis() - elapsedTime) > 30000) {
    rssi = rx.getRssi();
    snr = rx.getSnr();

    if ((lastRssi != rssi) || (lastSnr != snr) || (lastStereo != stereo)) {
      showStatus();
      lastRssi = rssi;
      lastSnr = snr;
      lastStereo = stereo;
    }

    elapsedTime = millis();
  }

  if (rx.getRdsReady()) {
    processRdsInfo();
  }
  delay(50);
}
