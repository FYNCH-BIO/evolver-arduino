#include <Tlc5940.h>

void setup()
{
  SerialUSB.begin(9600);
  Tlc.init(LEFT_PWM,4095); // initialise TLC5940 and set all channels off
}
 
void loop()
{


  for (int i = 0; i < 32; i = i + 1) {
    Tlc.set(LEFT_PWM,i, 0);
  
    Tlc.update();
    delay(50);
    
    Tlc.set(LEFT_PWM,i, 4095);
  
    Tlc.update();
    delay(50);
  }
    
}

