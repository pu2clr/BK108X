#include <BK108X.h>


#define SDA_PIN A4
#define CLK_PIN A5

BK108X rx;

void setup()
{
  Serial.begin(9600);
  while (!Serial);

  Serial.println("\n********Begin!");

 
  delay(2000);

  rx.setup(SDA_PIN, CLK_PIN);

  // rx.setFM(6400, 10800, 10650, 100);
  rx.setVolume(20);
  //delay(10000);
  rx.setAM(520, 1700, 810, 10);
  delay(2000);
  for (uint16_t i = 0; i < 100; i++) {
    rx.setChannel(i);
    Serial.println(i);
    delay(1000);
  }
 
  while(1);  
  getDeviceInfo(); 

  /*
  Serial.println("\nChecking the Volume.");
  for (int i = 31; i > 3; i-=3) {
    rx.setVolume(i);
    delay(500);
  } */

  delay(5000);
  
  rx.setVolume(15);
    
 
  Serial.println(rx.getRegister(REG02),BIN);


  delay(5000);
  
  Serial.println("\nPower Down!");
  rx.powerDown();
  Serial.println(rx.getRegister(REG02),BIN);

  delay(5000);
  Serial.println("\nPower UP!");
  Serial.println("You should hear a pop in the speaker.");
  rx.powerUp();
  Serial.println(rx.getRegister(REG02),BIN);
  Serial.println("\nFinish!***********");  
  rx.setFrequency(9550);

  delay(5000);
  Serial.println("\nTest");  
  for (uint16_t i = 0; i < 800; i+=10 ) {
    rx.setChannel(i);
    Serial.println(i);
    delay(600);
  }

  
}

void loop()
{

}


void getDeviceInfo() {

  char aux[100];
  uint16_t deviceId = rx.getDeviceId();
  uint16_t chipId = rx.getChipId();

  if ( (chipId ==0 || chipId == 0xFFFF)  || ( deviceId == 0 || deviceId == 0xFFFF )  ) {
    Serial.println("The device may not have been successfully detected. Please, check it.");
  }

  sprintf(aux, "Device Id = %u; Chip Id = %u", deviceId, chipId);
  Serial.println(aux);
  
}
