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
  
  Serial.println("\nFinish!***********");  

  
}

void loop()
{

}


void getDeviceInfo() {
   char aux[100];
    
  sprintf(aux,"Device Id = %d; Chip Id = %d",rx.getDeviceId(), rx.getChipId() );
  Serial.println(aux);

}
