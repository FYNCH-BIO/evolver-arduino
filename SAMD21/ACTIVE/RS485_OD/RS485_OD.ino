#include <evolver_si.h>
#include <Tlc5940.h>

// String Input
String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
boolean serialAvailable = true;  // if serial port is ok to write on

// Mux Shield Components and Control Pins
int s0 = 7, s1 = 8, s2 = 9, s3 = 10, SIG_pin = 0;
int num_vials = 16;
int mux_readings[16]; // The size Assumes number of vials
String comma = ",";
String data = "turb";
String end_mark = "end";

// Evolver Inputs
evolver_si in("we", " !", num_vials);

void setup() {
  pinMode(12, OUTPUT);
  digitalWrite(12, LOW);


  // Set up Mux
  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
  pinMode(s3, OUTPUT);

  digitalWrite(s0, LOW);
  digitalWrite(s1, LOW);
  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);

  analogReadResolution(16);
  Serial1.begin(9600);
  SerialUSB.begin(9600);
  // reserve 1000 bytes for the inputString:
  inputString.reserve(1000);
  Tlc.init(LEFT_PWM,4095);

  while (!Serial1);
}

void loop() {
  serialEvent(1);
  if (stringComplete) {
    SerialUSB.println(inputString);
    in.analyzeAndCheck(inputString);

    if (in.addressFound) {
      if (in.input_array[0] == "sf") {
        serialAvailable = false;
      }
      else if (in.input_array[0] == "st") {
        serialAvailable = true;
      }
      else {
        SerialUSB.println("Updating Values!");
        read_MuxShield();;

      }
      in.addressFound = false;
    }
    inputString = "";
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
    delay(1);
  }

}

void read_MuxShield() {
  int times_avg = 100;
  unsigned long mux_total[num_vials];
  int mux_readings[num_vials];

  memset(mux_total,0,sizeof(mux_total));
  for (int n=0; n < num_vials; n++){
    Tlc.set(LEFT_PWM,n, 4095 - in.input_array[n].toInt());
    while(Tlc.update());
    serialEvent(10);
    for (int h=0; h<(times_avg); h++){
      mux_total[n] = mux_total[n] + readMux(n);
    }
    mux_readings[n] = mux_total[n] / times_avg;
    //Tlc.set(LEFT_PWM,n, 4095);
    //while(Tlc.update());
  }
  digitalWrite(12, HIGH);
  String outputString = data;
  for (int n = 0; n < num_vials; n++) {
    outputString += mux_readings[n];
    outputString += comma;
  }
  outputString += end_mark;
  if (serialAvailable) {
    Serial1.println(outputString);
  }
  digitalWrite(12, LOW);
}

int readMux(int channel){
  int controlPin[] = {s0, s1, s2, s3};

  int muxChannel[16][4]={
    {0, 0, 0, 0}, //channel 0; Vial 1
    {1, 1, 0, 0}, //channel 3; Vial 2
    {1, 0, 0, 0}, //channel 1; Vial 3
    {0, 1, 0, 0}, //channel 2; Vial 4
    {0, 0, 1, 0}, //channel 4; Vial 5
    {1, 1, 1, 0}, //channel 7; Vial 6
    {1, 0, 1, 0}, //channel 5; Vial 7
    {0, 1, 1, 0}, //channel 6; Vial 8
    {0, 0, 0, 1}, //channel 8; Vial 9
    {1, 1, 0, 1}, //channel 11; Vial 10
    {1, 0, 0, 1}, //channel 9; Vial 11
    {0, 1, 0, 1}, //channel 10; Vial 12
    {0, 0, 1, 1}, //channel 12; Vial 13
    {1, 1, 1, 1}, //channel 15; Vial 14
    {1, 0, 1, 1}, //channel 13; Vial 15
    {0, 1, 1, 1}, //channel 14; Vial 16
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
