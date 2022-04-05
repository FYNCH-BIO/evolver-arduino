#include <Tlc5940.h>
#include <PID_v1.h>
#include <evolver_si.h>

boolean stringComplete = false;
boolean newInput = false;
String inputString = "";
const int numChannels = 8;
String address = "pres";
const String comma = ",";
const String end_mark = "end";
evolver_si in("pres", "_!", numChannels + 1);

float V_SOURCE = 3.3;

// Mux Control Pins
int s0 = 7, s1 = 8, s2 = 9, s3 = 10, SIG_PIN = 0;

double Input[8];
double Output[8];
double Setpoint[] = {0,0,0,0,0,0,0,0};
double savedSetpoint[] = {0,0,0,0,0,0,0,0};

double digitalVoltage;
double analogVoltage;
double pressure;

unsigned long sensorReadPeriod_us = 1;
unsigned long controlLoopPeriod_ms = 1;
unsigned long sensorLastReadTime_us;
unsigned long lastControlTime_ms;

// SET A MAX FOR THE PWM
int maxPWM = 4095;

float Kp = 1.5;
float Ki = 3.5;
float Kd = 0.00005;

int pressureindex = -1;
unsigned long lastChange;

PID pid1(&Input[0], &Output[0], &Setpoint[0], Kp, Ki, Kd, DIRECT);
PID pid2(&Input[1], &Output[1], &Setpoint[1], Kp, Ki, Kd, DIRECT);
PID pid3(&Input[2], &Output[2], &Setpoint[2], Kp, Ki, Kd, DIRECT);
PID pid4(&Input[3], &Output[3], &Setpoint[3], Kp, Ki, Kd, DIRECT);
PID pid5(&Input[4], &Output[4], &Setpoint[4], Kp, Ki, Kd, DIRECT);
PID pid6(&Input[5], &Output[5], &Setpoint[5], Kp, Ki, Kd, DIRECT);
PID pid7(&Input[6], &Output[6], &Setpoint[6], Kp, Ki, Kd, DIRECT);
PID pid8(&Input[7], &Output[7], &Setpoint[7], Kp, Ki, Kd, DIRECT);
PID *allPIDS[8] = {&pid1, &pid2, &pid3, &pid4, &pid5, &pid6, &pid7, &pid8};

void setup() {
  analogReadResolution(12);
  SerialUSB.begin(9600);
  Serial1.begin(9600);
  pinMode(A0, INPUT);
  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
  pinMode(s3, OUTPUT);
  pinMode(12, OUTPUT);

  
  digitalWrite(s0, LOW);
  digitalWrite(s1, LOW);
  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);

  sensorLastReadTime_us = micros();  
  lastControlTime_ms = millis();
  lastChange = millis();


  for (int i = 0; i < numChannels; i++) {
    allPIDS[i]->SetOutputLimits(-1, 1);
    allPIDS[i]->SetMode(AUTOMATIC);
    allPIDS[i]->SetSampleTime(5);
  }  

  Tlc.init(LEFT_PWM, 4095);  
}

void loop() {
  // Read Serial Inputs and checks if it is the right addresses
  serialEvent();
  if (stringComplete) {
    in.analyzeAndCheck(inputString);
    if (in.addressFound) {
      if (in.input_array[0] == "i" || in.input_array[0] == "r") {
        SerialUSB.println("Saving Setpoints");
        for (int n = 0; n < numChannels; n++) {
          savedSetpoint[n] = in.input_array[n+1].toDouble();
        }
        SerialUSB.println("Echoing New Pressure Command");
        newInput = true;
        echoCommand();
        SerialUSB.println("Waiting for OK to execute...");
      }
      if (in.input_array[0] == "a" && newInput) {
        for (int i = 0; i < numChannels; i++) {
          Setpoint[i] = savedSetpoint[i];
        }
        SerialUSB.println("Command Executed");
        newInput = false;
      }
    }
    inputString = "";
    stringComplete = false;
    in.addressFound = false;
  }


  // Sensor reading and valve actuation
  if (micros() - sensorLastReadTime_us > sensorReadPeriod_us) {
    readMuxShield();
    sensorLastReadTime_us = micros();
  }
  if (millis() - lastControlTime_ms > controlLoopPeriod_ms) {
    lastControlTime_ms = millis();
    
    for (int i = 0; i < numChannels; i++) {
      if (Setpoint[i] == 0) {
        continue;
      }      
      allPIDS[i]->Compute();
      double setValue = Output[i];
      int value = round(abs(setValue)*4095);
      if (value > maxPWM) {
        value = maxPWM;
      }
      int pwmValue = 4095 - value;
      if (Setpoint[i] != 0) {
        if (setValue >= 0) {
          Tlc.set(LEFT_PWM, i*2, pwmValue);
          Tlc.set(LEFT_PWM, (i*2)+1, 4095);
        }
        else {
          Tlc.set(LEFT_PWM, i*2, 4095);
          Tlc.set(LEFT_PWM, (i*2)+1, pwmValue);
        }
      }
      else {
        Tlc.set(LEFT_PWM, (i * 2), 4095);
        Tlc.set(LEFT_PWM, (i * 2) + 1, 4095);
      }
    }
  }
}

void readMuxShield() {
  int timesAvg = 1;
  unsigned long muxTotal[numChannels];
  int muxReadings[numChannels];
  memset(muxTotal, 0, sizeof(muxTotal));

  for (int i = 0; i < timesAvg; i++) {
    for (int j = 0; j < numChannels; j++) {
      muxTotal[j] = muxTotal[j] + readMux(j);
      while(Tlc.update());
    }  
  }

  for (int i = 0; i < numChannels; i++) {
    muxReadings[i] = muxTotal[i] / timesAvg;
    Input[i] = calcPressure(muxReadings[i]);
    //SerialUSB.print(Input[i]);
    //SerialUSB.print('\t');
  }
  //SerialUSB.println();
}

double calcPressure(int reading) {
  digitalVoltage = (reading * V_SOURCE) / 4096;
  analogVoltage = (reading * V_SOURCE) / 4096;
  pressure = (15 * (analogVoltage - 0.1 * V_SOURCE)) / (0.8 * V_SOURCE);  
  return pressure;
}

int readMux(int channel) {
  int controlPin[] = {s0, s1, s2, s3};
  int muxChannel[8][4] = {
    {0, 0, 0, 0}, //channel 0; Vial 1
    {1, 1, 0, 0}, //channel 3; Vial 2
    {1, 0, 0, 0}, //channel 1; Vial 3
    {0, 1, 0, 0}, //channel 2; Vial 4
    {0, 0, 1, 0}, //channel 4; Vial 5
    {1, 1, 1, 0}, //channel 7; Vial 6
    {1, 0, 1, 0}, //channel 5; Vial 7
    {0, 1, 1, 0}, //channel 6; Vial 8
  };
  
  //loop through the 4 sig
  for (int i = 0; i < 4; i ++) {
    digitalWrite(controlPin[i], muxChannel[channel][i]);
  }

  //read the value at the SIG pin
  int val = analogRead(SIG_PIN);
  return val;
}


void serialEvent() {
  while (Serial1.available()) {
    char inChar = (char)Serial1.read();
    inputString += inChar;
    if (inChar == '!') {
      stringComplete = true;
      break;
    }
  }
}

void echoCommand() {
  digitalWrite(12, HIGH);
  
  String outputString = address + "e,";
  for (int n = 0; n < numChannels; n++) {
    outputString += in.input_array[n+1];
    outputString += comma;
  }
  outputString += end_mark;

  delay(100); // important to make sure pin 12 flips appropriately

  SerialUSB.println(outputString);
  Serial1.print(outputString);

  delay(100); // important to make sure pin 12 flips appropriately

  
  digitalWrite(12, LOW);
}
