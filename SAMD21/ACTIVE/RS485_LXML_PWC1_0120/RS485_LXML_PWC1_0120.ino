#include <evolver_si.h>
#include <Tlc5940.h>

// Input Variables Used
String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;

int num_vials = 16;
evolver_si in("px"," !", num_vials*2);

int Input[16];

void setup()
{

  Tlc.init(LEFT_PWM | RIGHT_PWM,4095);
  Serial1.begin(9600);
  SerialUSB.begin(9600);
  // reserve 200 bytes for the inputString:
  inputString.reserve(1000);
  while (!Serial1);

  for (int i = 0; i < num_vials; i++) {
    Tlc.set(LEFT_PWM, i, 4095);
  }
  for (int i = 0; i < num_vials; i++) {
    Tlc.set(RIGHT_PWM, i, 4095);
  }
  while(Tlc.update());
  
}


void loop() {
  serialEvent(1);
  if (stringComplete) {
    SerialUSB.println(inputString);
    in.analyzeAndCheck(inputString);
    
    if(in.addressFound){
      SerialUSB.println("Address Found");
      for (int i = 0; i < num_vials; i++) {
        if (in.input_array[i] != "NaN") {
          Input[i] =  in.input_array[i].toInt();
          Tlc.set(LEFT_PWM, i, Input[i]);
        }
      }
      for (int i = 0; i < num_vials; i++) {
        if (in.input_array[i+num_vials] != "NaN") {
          Input[i] =  in.input_array[i+num_vials].toInt();
          Tlc.set(RIGHT_PWM, i, Input[i]);
        }
      }
      while(Tlc.update());

    }
    else {
      SerialUSB.println("Address Not Found");
    }
    inputString = "";
    stringComplete = false;
    in.addressFound = false;
  }
}



void serialEvent(int time_wait) {
  for (int n=0; n<time_wait; n++) {
      while (Serial1.available()) {
        char inChar = (char)Serial1.read();
        inputString += inChar;
        if (inChar == '!') {
          stringComplete = true;
        }
      }
    delay(1);
  }
  
}
