#include <evolver_si.h>

//DEBUG COMMENT: to activate, comment tcb,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,_! or tccr,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,_! 

// Serial Event Variables
String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
boolean serialAvailable = true;  // if serial port is ok to write on

// Mux Control Pins
int s0 = 7, s1 = 8, s2 = 9, s3 = 10, SIG_PIN = 0;
int num_vials = 16;

// Mux Shield Values
String address = "tempcal";
String comma = ",";
String output_string = "";
String end_mark = "end";

// Input Control
evolver_si in("tempcal", "_!", num_vials+1);

// PID Settings
double TemperatureReadings[] = {4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095};


//each thermristor must be calibrated to give the appropriate voltage-to-temperature curve.
//these are the coefficients, saved to flash so they persist
float linearCalibrationCoefficientM[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
float linearCalibrationCoefficientB[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

float voltageToTemperature(float voltage, int thermristorIndex){
  // y = mx + b
  float m = linearCalibrationCoefficientM[thermristorIndex];
  float b = linearCalibrationCoefficientB[thermristorIndex];
  return m*voltage+b;
}


void setup() {
  pinMode(12, OUTPUT);
  digitalWrite(12, LOW);
  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
  pinMode(s3, OUTPUT);

  digitalWrite(s0, LOW);
  digitalWrite(s1, LOW);
  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);

  analogReadResolution(12);

  // initialize serial:
  Serial1.begin(9600);
  SerialUSB.begin(9600);
  SerialUSB.println("Beginning");
  // reserve 1000 bytes for the inputString:
  inputString.reserve(1000);

  
    SerialUSB.println("Beginning");
    delay(1000);

  while (!Serial1){

    
    SerialUSB.println("Beginning");
  }
}

void loop() {
  serialEvent();
  if (stringComplete){
    in.analyzeAndCheck(inputString);

    if (in.addressFound && (in.input_array[0] == "i" || in.input_array[0] == "r")) {
      SerialUSB.println("Starting");
      print_thermometer_readings();
      
      if (in.input_array[1].length() >= 2) {//calibration settings: cr = 'calibration read' and cs is 'calibration set'
        String action  = in.input_array[1];
        if (action == "cr") { //'cr'
          SerialUSB.println("please no");
          print_calibration_coefficients();
        }
        if (action == "csetm") { //calibration set "m" coefficients (slope)
          in.input_array[1]; //chop off the 'cs' from the beginning of the string
          SerialUSB.println("please no");
          set_calibration_coefficients_m();
        }
        if (action == "csetb") { //'calibration set "m" coefficients (slope)
          in.input_array[1]; //chop off the 'cs' from the beginning of the string
          SerialUSB.println("please no");
          set_calibration_coefficients_b();
        }
      }
      
      
      if (in.input_array[0] == "sf") {
        serialAvailable = false;
      }
      in.addressFound = false;
    }
    inputString = "";
  }
  // clear the string:
  stringComplete = false;
  read_MuxShield();
}

void print_calibration_coefficients(){
  String outputString = address+"b,"; //b for broadcast
  for (int i = 0; i < num_vials; i += 1) {
    outputString += String((int)linearCalibrationCoefficientM[i]) + comma;
  }
  outputString += "|";
  for (int i = 0; i < num_vials; i += 1) {
    outputString += String((int)linearCalibrationCoefficientB[i]) + comma;
  }
  outputString += end_mark;
  Serial1.println(outputString);
  SerialUSB.println(outputString);
}

void set_calibration_coefficients_m() {
  for (int i = 0; i < num_vials; i = i + 1) {
    if (in.input_array[i] != "NaN") {
      //input_array[0] is "cm" here, so index i+1 is grabbed
      linearCalibrationCoefficientM[i] = (double)in.input_array[i+2].toFloat();
    }
  }
}

void set_calibration_coefficients_b() {
  for (int i = 0; i < num_vials; i = i + 1) {
    if (in.input_array[i] != "NaN") {
      //input_array[0] is "cm" here, so index i+1 is grabbed
      linearCalibrationCoefficientB[i] = (double)in.input_array[i+2].toFloat();
    }
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
    //allow debug input
    while (SerialUSB.available()) {
      char inChar = (char)SerialUSB.read();
      inputString += inChar;
      if (inChar == '!') {
        stringComplete = true;
      }
    }
}


void read_MuxShield() {

  int times_avg = 3;
  unsigned long mux_total[num_vials];
  int mux_readings[num_vials];

  memset(mux_total, 0, sizeof(mux_total));
  for (int h = 0; h < (times_avg); h++) {
    for (int n = 0; n < num_vials; n++) {
      mux_total[n] = (mux_total[n] + readMux(n));
    }
  }

  for (int m = 0; m < num_vials; m++) {
    mux_readings[m] = mux_total[m] / times_avg;
    TemperatureReadings[m] = mux_readings[m];
  }
}



void print_thermometer_readings(){
  digitalWrite(12, HIGH);
  SerialUSB.println("thermoreading");

  String intro = address + "b,";//b for broadcast
  
  Serial1.print(intro);
  SerialUSB.print(intro); 

  for (int i = 0; i < 16; i = i + 1) {

   String tempreading = String(TemperatureReadings[i]) + comma;
   SerialUSB.print(i);
   
   Serial1.print(tempreading);
   SerialUSB.print(tempreading);

  }

  
  Serial1.print(end_mark);
  SerialUSB.print(end_mark);
  digitalWrite(12, LOW);
}

int readMux(int channel) {
  int controlPin[] = {s0, s1, s2, s3};

  int muxChannel[16][4] = {
    {0, 0, 0, 0}, //channel 0; Probe 1
    {1, 1, 0, 0}, //channel 3; Probe 2
    {1, 0, 0, 0}, //channel 1; Probe 3
    {0, 1, 0, 0}, //channel 2; Probe 4
    {0, 0, 1, 0}, //channel 4; Probe 5
    {1, 1, 1, 0}, //channel 7; Probe 6
    {1, 0, 1, 0}, //channel 5; Probe 7
    {0, 1, 1, 0}, //channel 6; Probe 8
    {0, 0, 0, 1}, //channel 8; Probe 9
    {1, 1, 0, 1}, //channel 11; Probe 10
    {1, 0, 0, 1}, //channel 9; Probe 11
    {0, 1, 0, 1}, //channel 10; Probe 12
    {0, 0, 1, 1}, //channel 12; Probe 13
    {1, 1, 1, 1}, //channel 15; Probe 14
    {1, 0, 1, 1}, //channel 13; Probe 15
    {0, 1, 1, 1}, //channel 14; Probe 16
  };

  //loop through the 4 sig
  for (int i = 0; i < 4; i ++) {
    digitalWrite(controlPin[i], muxChannel[channel][i]);
  }

  //read the value at the SIG pin
  int val = analogRead(SIG_PIN);

  //return the value
  return val;
}
