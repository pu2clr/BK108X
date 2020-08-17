#include <BK108X.h>

BK108X rx;

void setup()
{
  Serial.begin(9600);
  while (!Serial);

  Serial.println("\n********Before!");


  pinMode(9,OUTPUT);
  digitalWrite(9, LOW);
  
  delay(2000);

  digitalWrite(9, HIGH);
  delay(1000);
  checkDevice();   

  rx.setup();
  Serial.println("\nAfter!***********");  

  checkDevice();
  
}

void loop()
{

}

void checkDevice() {
  char aux[100];
  checkI2C();
  if ( !checkI2C() ) {
    
    Serial.println("Check your circuit!");
    while (1);
  }

  Serial.println("Device Found!!!");

  sprintf(aux,"Device Id = %d; Chip Id = %d",rx.getDeviceId(), rx.getChipId() );
  Serial.println(aux);
}


bool checkI2C() {
  byte error, address;
  int nDevices;
  Serial.println("I2C bus Scanning...\n");
  Wire.begin();
  nDevices = 0;
  for (address = 1; address < 127; address++ ) {
    Serial.println(address, HEX);
    delay(1);
    Wire.beginTransmission(address);
    delay(1);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("\nI2C device found at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.println(address, HEX);
      nDevices++;
    }
    else if (error == 4) {
      Serial.print("\nUnknow error at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
    return false;
  }
  else {
    Serial.println("done\n");
    return true;
  }
}
