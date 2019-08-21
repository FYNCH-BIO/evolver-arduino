// Recurring Command: "stirr,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,_!"
// Immediate Command: "stiri,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,_!"
// Acknowledgement to Run: "stira,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,_!"

#include <evolver_si.h>
#include <Tlc5940.h>

// Input Variables Used
String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;

// Serial Communication Variables
String comma = ",";
String end_mark = "end";
int num_vials = 16;
String address = "stir";
evolver_si in("stir","_!", num_vials+1); // 17 CSV-inputs from RPI
boolean new_input = false;
int saved_inputs[] = {8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8};

int Input[] = {8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8};

void setup()
{

  Tlc.init(LEFT_PWM,4095); // initialise TLC5940 and set all channels off
  SerialUSB.begin(9600);
  Serial1.begin(9600);
  // reserve 200 bytes for the inputString:
  inputString.reserve(1000);
  while (!Serial1);
  pinMode(12, OUTPUT);
  digitalWrite(12, LOW);

}


void loop() {
  serialEvent(1);
  if (stringComplete) {
    SerialUSB.println(inputString);
    in.analyzeAndCheck(inputString);

    if(in.addressFound){
      if (in.input_array[0] == "i" || in.input_array[0] == "r") {

        SerialUSB.println("Saving Setpoints");
        for (int n = 1; n < num_vials+1; n++) {
          saved_inputs[n-1] = in.input_array[n].toInt();
        }

        SerialUSB.println("Echoing New Stir Command");
        new_input = true;
        echoCommand();

        SerialUSB.println("Waiting for OK to execute...");
      }

      if (in.input_array[0] == "a" && new_input) {
        update_values();
        SerialUSB.println("Command Executed!");
        new_input = false;
      }

      inputString = "";

    }

    //Clears strings if too long
    if (inputString.length() > 900){
      SerialUSB.println("Cleared Input String");
      inputString = "";
    }

    stringComplete = false;
    in.addressFound = false;
  }
  exec_stir();
}

void echoCommand() {
  digitalWrite(12, HIGH);

  String outputString = address + "e,";
  for (int n = 0; n < num_vials; n++) {
    outputString += saved_inputs[n];
    outputString += comma;
  }
  outputString += end_mark;
  SerialUSB.println(outputString);
  Serial1.print(outputString); // issues w/ println on Serial 1 being read into Raspberry Pi
  delay(300); // important to make sure pin 12 flips appropriately

  digitalWrite(12, LOW);
}


void update_values() {
  for (int i = 0; i < num_vials; i++) {
     Input[i] =  saved_inputs[i];
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
