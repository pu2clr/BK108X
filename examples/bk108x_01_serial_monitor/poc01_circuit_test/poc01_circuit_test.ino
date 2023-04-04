#include <BK108X.h>


#define SDA_PIN 4
#define CLK_PIN 5


#define STRONG_LOCAL_FM_STATION 10650   // Set the best FM frequency to your location. The current is 106.50MHz 
#define STRONG_LOCAL_AM_STATION 620     // Set the best AM freqiency to your location. The current is 620KHz 

BK108X rx;

void setup()
{
  Serial.begin(9600);
  while (!Serial);

  Serial.println("\n********Begin!");
 
  delay(1000);

  rx.setup(SDA_PIN, CLK_PIN);
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

  // If you are using BK1088E, the Chip ID should be 4224 (0x1080) 
  sprintf(aux, "Device Id = %u; Chip Id = %u", deviceId, chipId);
  Serial.println(aux);
  
}
