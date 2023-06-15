#include <BK108X.h>
#include <Wire.h>

#define SDA_PIN A4
#define CLK_PIN A5


#define STRONG_LOCAL_FM_STATION 10650   // Set the best FM frequency to your location. The current is 106.50MHz 
#define STRONG_LOCAL_AM_STATION 620     // Set the best AM freqiency to your location. The current is 620KHz 

BK108X rx;

char bufferAux[160];


void setup()
{

  uint8_t i2cBusList[15];
  int i2cStatus;

  Serial.begin(9600);
  while (!Serial);

  /*
  i2cStatus = rx.checkI2C(i2cBusList); 

  if (i2cStatus == -1)
    showMsgHalt(F("\nError while try to access the device.\n")); 
  else if (i2cStatus == 0) 
    showMsgHalt(F("\nNo device was detected. Please, check your circuit!"));
  else { 
    for (int i = 0; i < i2cStatus ; i++) {
      sprintf(bufferAux,"Found I2C adress: %X (HEX)\n", i2cBusList[i]);
      Serial.print(bufferAux);
    }
  }    */

  Serial.println("\n********Begin!");
 
  delay(1000);

  Serial.println("\nStarting the receiver!");

  rx.setup(SDA_PIN, CLK_PIN);

  showRegisters();
  
  Serial.println("\nReceiver started!");
  
  getDeviceInfo(); 
  
  rx.setVolume(20);       

  Serial.print("\nChecking an AM station in your location at ");
  Serial.print(STRONG_LOCAL_AM_STATION);
  Serial.print("KHZ\n\n");
  rx.setAM(520, 1700, STRONG_LOCAL_AM_STATION, 10);     
  delay(10000);

  Serial.print("\nChecking an FM station in your location at ");
  Serial.print(STRONG_LOCAL_FM_STATION / 100.0);
  Serial.print("MHZ\n\n");
  rx.setFM(6400, 10800, STRONG_LOCAL_FM_STATION, 100);
  delay(10000);
   rx.setFrequency(10390);
 
  Serial.println("\nFinish!***********");  
  
}

void showRegisters() {

  uint16_t *regContent;
  uint8_t reg;
  uint16_t aux;
  char bufferAux[60];
  Serial.println("\n***Register values***** \n");
  regContent = rx.getRegisterValues();
  for ( reg = 0; reg < 32; reg++) {
    aux =  regContent[reg];
    sprintf(bufferAux,"Reg %d: %x (%u)", reg, aux, aux );
    Serial.println(bufferAux);
  }

}


void loop()
{

}


void showMsgHalt(String msg) {
  Serial.print(F("\n********************\n"));
  Serial.print(msg);
  Serial.print(F("\n********************\n")); 
  while(1);
}

void getDeviceInfo() {

  char aux[100];
  uint16_t deviceId = rx.getDeviceId();
  uint16_t chipId = rx.getChipId();

  if ( (chipId ==0 || chipId == 0xFFFF)  || ( deviceId == 0 || deviceId == 0xFFFF )  ) {
    Serial.println("The device may not have been successfully detected. Please, check it.");
  }

  // If you are using BK1088E, the Chip ID should be 4224 (0x1080) 
  sprintf(aux, "Device Id = %u; Chip Id = %u", deviceId, chipId);
  Serial.println(aux);
  
}






