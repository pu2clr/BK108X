/**
 * Circuit test 
 * Use this sketch with the Arduino IDE and Serial Monitor.
 * This sketch can be useful to test your circuit with BK1086 or BK1088.
 * Active the Serial Monitor on your Arduino IDE.
 * 
 * This sketch can be freely distributed using the MIT Free Software model.
 * Copyright (c) 2020 Ricardo Lima Caratti.  
 * Contact: pu2clr@gmail.com
 */

BK108X rx;

void setup()
{
  char aux[100];
  Serial.begin(9600);
  while (!Serial);
  checkI2C();
  if ( !checkI2C() ) {
    
    Serial.println("Check your circuit!");
    while (1);
  }

  Serial.println("Device Found!!!");

  sprintf(aux,"Device Id = %d; Chip Id = %d",rx.getRegister(REG00), rx.getRegister(REG01) );
  Serial.println(aux);
  

  rx.setup();
}

void loop()
{

}


bool checkI2C() {
  byte error, address;
  int nDevices;
  Serial.println("I2C bus Scanning...\n");
  nDevices = 0;
  for (address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
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