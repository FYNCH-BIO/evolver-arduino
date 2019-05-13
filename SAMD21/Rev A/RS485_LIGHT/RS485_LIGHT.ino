#include <evolver_si.h>
#include <Tlc5940.h>

// Input Variables Used
String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;

int num_vials = 16;
evolver_si in("jw"," !", num_vials*2);

int Input[16];
double Setpoint[] = {4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095};

void setup()
{

  Tlc.init(LEFT_PWM | RIGHT_PWM,4095);
  Serial1.begin(9600);
  SerialUSB.begin(9600);
  // reserve 200 bytes for the inputString:
  inputString.reserve(1000);
  while (!Serial1);
  
}


void loop() {
  serialEvent();
  if (stringComplete) {
    SerialUSB.println(inputString);
    in.analyzeAndCheck(inputString);
    
    if(in.addressFound){
      SerialUSB.println("Address Found");
      for (int i = 0; i < num_vials; i++) {
        Input[i] =  in.input_array[i].toInt();
        Setpoint[i] = Input[i];
      }
      for (int i = 0; i < num_vials; i++) {
        Input[i] =  in.input_array[i+num_vials].toInt();
        Setpoint[i+num_vials] = Input[i];
      }

    }
    else {
      SerialUSB.println("Address Not Found");
    }
    for (int i = 0; i < num_vials; i++) {
      Tlc.set(LEFT_PWM, i, Setpoint[i]);
      Tlc.set(RIGHT_PWM, i, Setpoint[i+num_vials]); 
    }
    while(Tlc.update());    
    inputString = "";
    stringComplete = false;
    in.addressFound = false;
  }
}


void serialEvent() {
    while (Serial1.available()) {
      char inChar = (char)Serial1.read();
      inputString += inChar;
      if (inChar == '!') {
        stringComplete = true;
      }
    }
  
}
