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
  
  getDeviceInfo(); 

  rx.setVolume(30);
  
  Serial.println(rx.getRegister(REG02),BIN);


  delay(4000);
  
  Serial.println("\nPower Down!");
  rx.powerDown();
  Serial.println(rx.getRegister(REG02),BIN);
  delay(4000);
  Serial.println("\nPower UP!");
  Serial.println("You should hear a pop in the speaker.");
  rx.powerUp();
  Serial.println(rx.getRegister(REG02),BIN);
  Serial.println("\nFinish!***********");  

  
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
