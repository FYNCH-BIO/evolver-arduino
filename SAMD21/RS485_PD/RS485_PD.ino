// Recurring Command: 'od_90r,1000,_!'
// Immediate Command: 'od_90i,1000,_!'
// Acknowledgement to Run: 'od_90a,1000,_!'


#include <evolver_si.h>

// String Input
String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete

// Mux Shield Components and Control Pins
int s0 = 2, s1 = 3, s2 = 4, s3 = 5, SIG_pin = A1;
int num_vials = 16;
int mux_readings[16]; // The size Assumes number of vials
int active_vial = 0;
int PDtimes_averaged = 1000;
int output[] = {60000,60000,60000,60000,60000,60000,60000,60000,60000,60000,60000,60000,60000,60000,60000,60000};

// Evolver Inputs
String comma = ",";
String end_mark = "end";
int expected_PDinputs = 2;
String photodiode_address = "od_90";
evolver_si in("od_90", "_!", expected_PDinputs);//2 CSV Inputs from RPI
boolean new_PDinput = false;
int saved_PDaveraged = 1000; // saved input from Serial Comm.

void setup() {
  pinMode(12, OUTPUT);
  digitalWrite(12, LOW);


  // Set up Mux
  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
  pinMode(s3, OUTPUT);
  pinMode(SIG_pin, INPUT);

  digitalWrite(s0, LOW);
  digitalWrite(s1, LOW);
  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);

  analogReadResolution(16);
  Serial1.begin(9600);
  SerialUSB.begin(9600);
  // reserve 1000 bytes for the inputString:
  inputString.reserve(1000);
  while (!Serial1);
}

void loop() {
  SerialUSB.print("Reading Vial:");
  SerialUSB.println(active_vial);
  read_MuxShield();;
  if (stringComplete) {
    SerialUSB.println(inputString);
    in.analyzeAndCheck(inputString);

    // Photodiode Logic
    if (in.addressFound) {
      if (in.input_array[0] == "i" || in.input_array[0] == "r") {
        
        SerialUSB.println("Saving PD Setting");
        saved_PDaveraged = in.input_array[1].toInt();
        
        SerialUSB.println("Echoing New PD Command");
        new_PDinput = true;
        dataResponse();

        SerialUSB.println("Waiting for OK to execute...");
      }
      if (in.input_array[0] == "a" && new_PDinput) {
        PDtimes_averaged = saved_PDaveraged;
        SerialUSB.println("PD Command Executed!");
        new_PDinput = false;
      }        
      
      in.addressFound = false;
      inputString = "";
    }
    
    //Clears strings if too long
    if (inputString.length() >900){
      SerialUSB.println("Cleared Input String");
      inputString = "";
    }
  }

  // clear the string:
  stringComplete = false;
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
  }
}

int dataResponse (){
  digitalWrite(12, HIGH);
  String outputString = photodiode_address + "b,"; // b is a broadcast tag
  for (int n = 0; n < num_vials; n++) {
    outputString += output[n];
    outputString += comma;
  }
  outputString += end_mark;
  
  delay(100); // important to make sure pin 12 flips appropriately
  
  SerialUSB.println(outputString);
  Serial1.print(outputString); // issues w/ println on Serial 1 being read into Raspberry Pi

  delay(100); // important to make sure pin 12 flips appropriately
  
  digitalWrite(12, LOW);
}

void read_MuxShield() {
  unsigned long mux_total=0;
  
  for (int h=0; h<(PDtimes_averaged); h++){
    mux_total = mux_total + readMux(active_vial);
    serialEvent(1);
    if (stringComplete){
      SerialUSB.println("String Completed, stop averaging");
      SerialUSB.println(h);
      break;
    }
  }
  if (!stringComplete){
    output[active_vial] = mux_total / PDtimes_averaged;
    SerialUSB.println(output[active_vial]);
    if (active_vial == 15){
      active_vial = 0;
    } else {
      active_vial++;
    }
  }
}


int readMux(int channel){
  int controlPin[] = {s0, s1, s2, s3};

  int muxChannel[16][4]={
    {0, 0, 0, 0}, //channel 0; Vial 0
    {1, 1, 0, 0}, //channel 3; Vial 1
    {1, 0, 0, 0}, //channel 1; Vial 2
    {0, 1, 0, 0}, //channel 2; Vial 3
    {0, 0, 1, 0}, //channel 4; Vial 4
    {1, 1, 1, 0}, //channel 7; Vial 5
    {1, 0, 1, 0}, //channel 5; Vial 6
    {0, 1, 1, 0}, //channel 6; Vial 7
    {0, 0, 0, 1}, //channel 8; Vial 8
    {1, 1, 0, 1}, //channel 11; Vial 9
    {1, 0, 0, 1}, //channel 9; Vial 10
    {0, 1, 0, 1}, //channel 10; Vial 11
    {0, 0, 1, 1}, //channel 12; Vial 12
    {1, 1, 1, 1}, //channel 15; Vial 13
    {1, 0, 1, 1}, //channel 13; Vial 14
    {0, 1, 1, 1}, //channel 14; Vial 15
  };

  //loop through the 4 sig
  for(int i = 0; i < 4; i ++){
    digitalWrite(controlPin[i], muxChannel[channel][i]);
  }

  //read the value at the SIG pin
  int val = analogRead(SIG_pin);

  //return the value
  return val;
}
