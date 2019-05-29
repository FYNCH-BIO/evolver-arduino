// Example Photodidoe Command: 'wec,1000, !'
// Example Photodiode Broadcast: 'web,1000, !'

// Example LED Command: 'ldc,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095, !"

#include <evolver_si.h>
#include <Tlc5940.h>

// String Input
String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
boolean serialAvailable = true;  // if serial port is ok to write on

// Mux Shield Components and Control Pins
int s0 = 7, s1 = 8, s2 = 9, s3 = 10, SIG_pin = A0;
int num_vials = 16;
int mux_readings[16]; // The size Assumes number of vials
String comma = ",";
String end_mark = "end";
int active_vial = 0;
int times_averaged = 1000;
int output[] = {60000,60000,60000,60000,60000,60000,60000,60000,60000,60000,60000,60000,60000,60000,60000,60000};
int Input[] = {4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095};

// Evolver Inputs
int expected_inputs = 2;
String photodiode_address = "we";
// Photodiode Measurements
evolver_si in("we", " !", expected_inputs); //3rd Input is same as expected_inputs
// LED Settings
String led_address = "ld";
evolver_si led("ld", " !", num_vials+1); //3rd Input is same as expected_inputs



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
  Tlc.init(LEFT_PWM,4095);
  Serial1.begin(9600);
  SerialUSB.begin(9600);
  // reserve 1000 bytes for the inputString:
  inputString.reserve(1000);
  while (!Serial1);

  for (int i = 0; i < num_vials; i++) {
    Tlc.set(LEFT_PWM, i, 4095 - Input[i]);
  }
  while(Tlc.update());

}

void loop() {
  SerialUSB.print("Reading Vial:");
  SerialUSB.println(active_vial);
  read_MuxShield();;
  if (stringComplete) {
    SerialUSB.println(inputString);
    in.analyzeAndCheck(inputString);
    led.analyzeAndCheck(inputString);

    if (in.addressFound) {
      times_averaged = in.input_array[1].toInt();
      
      if (in.input_array[0] == "c") {
        SerialUSB.println("Echoing PD command!");
        echoPhotodiode();
      }
      else if (in.input_array[0] == "b") {
        SerialUSB.println("Outputting PD Data for Broadcast!");
        dataResponse();
      }

      in.addressFound = false;
      inputString = "";
    }

    if (led.addressFound) {
      for (int i = 1; i < num_vials+1; i++) {
        Tlc.set(LEFT_PWM, i, 4095 - led.input_array[i].toInt());
      }
      while(Tlc.update());
      
      if (led.input_array[0] == "c") {
        SerialUSB.println("Echoing LED command!");
        echoLED();
      }
      led.addressFound = false;
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

void echoPhotodiode() {
  digitalWrite(12, HIGH);
  
  String outputString = photodiode_address + "e,";
  for (int n = 1; n < expected_inputs; n++) {
    outputString += in.input_array[n];
    outputString += comma;
  }
  outputString += end_mark;
  if (serialAvailable) {
    SerialUSB.println(outputString);
    Serial1.println(outputString);
  }
  
  digitalWrite(12, LOW);
}

void echoLED() {
  digitalWrite(12, HIGH);
  
  String outputString = led_address + "e,";
  for (int n = 1; n < num_vials+1 ; n++) {
    outputString += led.input_array[n];
    outputString += comma;
  }
  outputString += end_mark;
  if (serialAvailable) {
    SerialUSB.println(outputString);
    Serial1.println(outputString);
  }
  
  digitalWrite(12, LOW);
}


int dataResponse (){
  digitalWrite(12, HIGH);
  String outputString = photodiode_address + "d,";
  for (int n = 0; n < num_vials; n++) {
    outputString += output[n];
    outputString += comma;
  }
  outputString += end_mark;
  if (serialAvailable) {
    SerialUSB.println(outputString);
    Serial1.println(outputString);
  }
  digitalWrite(12, LOW);
}

void read_MuxShield() {
  unsigned long mux_total=0;
  
  for (int h=0; h<(times_averaged); h++){
    mux_total = mux_total + readMux(active_vial);
    serialEvent(1);
    if (stringComplete){
      SerialUSB.println("String Completed, stop averaging");
      SerialUSB.println(h);
      break;
    }
  }
  if (!stringComplete){
    output[active_vial] = mux_total / times_averaged;
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
