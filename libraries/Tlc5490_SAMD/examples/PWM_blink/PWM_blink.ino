/*
 * Example code to blink all of the channels connected to one or two PWM boards 
 * 
 * The last parameter in Tlc.init() and Tlc.Set() corresponds to the fraction of time the signal is on (set by the PWM)
 * (0 is fully on, 4095 is off. Any value in between is also valid.)
 * 
 * The commented sections each only blink the channels on one of the PWM boards
 * They exist to demonstrate how to adjust one board without changing the other
 */

#include <Tlc5940.h>

void setup()
{
  Tlc.init(LEFT_PWM | RIGHT_PWM,4095); // initialise both PWM boards and set all channels off
  
  //Tlc.init(LEFT_PWM,4095);  // initialise only the left PWM board and set all channels off
  
  //Tlc.init(RIGHT_PWM,4095); // initialise only the right PWM board and set all channels off
}
 
void loop()
{
  //Alternatively blink all the channels on the left PWM board, then right PWM board

  //Set all channels on the left PWM board to fully on, and all on the right PWM board off
  for (int i = 0; i < 16; i = i + 1) { //i corresponds to a channel on the PWM board that is being set
    Tlc.set(LEFT_PWM,i, 0);
    Tlc.set(RIGHT_PWM,i, 4095);
  }

  //update both PWMs (both have been set so both will change)
  Tlc.update();
  delay(1000); //1 second
  
  //Set all channels on the right PWM board to fully on, and all on the left PWM board off
  for (int i = 0; i < 16; i = i + 1) { //i corresponds to a channel on the PWM board that is being set
    Tlc.set(LEFT_PWM,i, 4095);
    Tlc.set(RIGHT_PWM,i, 0);
  }
  
  //update both PWMs (both have been set so both will change)
  Tlc.update();
  delay(1000); //1 second

  /*
  //Blink all the channels on the left PWM board, do not change the right PWM board at all

  
  //Set all channels on the left PWM board to fully on
  for (int i = 0; i < 16; i = i + 1) { //i corresponds to a channel on the PWM board that is being set
    Tlc.set(LEFT_PWM,i, 0);
  }

  //update both PWMs (only one has been set so only one will change)
  Tlc.update();
  delay(1000); // 1 second
  
  //Set all channels on the left PWM board to fully off
  for (int i = 0; i < 16; i = i + 1) { //i corresponds to a channel on the PWM board that is being set
    Tlc.set(LEFT_PWM,i, 4095);
  }
  
  //update both PWMs (only one has been set so only one will change)
  Tlc.update();
  delay(1000); // 1 second
  */

  /*
  //Blink all the channels on the left, do not change the left PWM board at all

  
  //Set all channels on the rgiht PWM board to fully off
  for (int i = 0; i < 16; i = i + 1) { //i corresponds to a channel on the PWM board that is being set
    Tlc.set(RIGHT_PWM,i, 4095);
  }
  
  //update both PWMs (only one has been set so only one will change)
  Tlc.update();
  delay(1000); // 1 second
  
  //Set all channels on the right PWM board to fully on
  for (int i = 0; i < 16; i = i + 1) { //i corresponds to a channel on the PWM board that is being set
    Tlc.set(RIGHT_PWM,i, 0);
  }
  
  //update both PWMs (only one has been set so only one will change)
  Tlc.update();
  delay(1000); // 1 second
  */
}

