#include <evolver_si.h>
#include <Tlc5940.h>

// Input Variables Used
String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;

String data = "stir";
String end_mark = "end";
int num_vials = 16;
evolver_si in("zv"," !", num_vials);

int Input[] = {8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8};

void setup()
{

  Tlc.init(LEFT_PWM,4095); // initialise TLC5940 and set all channels off
  SerialUSB.begin(9600);
  Serial1.begin(9600);
  // reserve 200 bytes for the inputString:
  inputString.reserve(1000);
  while (!Serial1);
  digitalWrite(12, LOW);
  
}


void loop() {
  serialEvent(1);
  if (stringComplete) {
    
    in.analyzeAndCheck(inputString);
    
    if(in.addressFound){
      SerialUSB.println("Found");
      update_values();
      
    } else {
      SerialUSB.println("Address Not Found");
    }
    
    inputString = "";
    stringComplete = false;
    in.addressFound = false;
  }
  exec_stir();
}

void update_values() {
  for (int i = 0; i < num_vials; i++) {
    if (in.input_array[i] != "NaN") {
      Input[i] =  in.input_array[i].toInt();
    }
  }
}

void exec_stir()
{
  for (int i = 0; i < num_vials; i++) {
    if (Input[i]  != 0) {
      Tlc.set(LEFT_PWM,i, 0);
      //Serial.print("Code: " + String(in.output_array[i]) + " ");
    }
  }
  //Serial.println();
  
    while(Tlc.update());
    serialEvent(12);
    
   // 10 settings for the stir rate
   for (int n = 0; n < 98; n++) { 
    for (int i = 0; i < num_vials; i++) {

      if (Input[i] == n) {
        Tlc.set(LEFT_PWM,i, 4095);
      }
    }
    while(Tlc.update());
    serialEvent(1);
   }

   serialEvent(70);
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
