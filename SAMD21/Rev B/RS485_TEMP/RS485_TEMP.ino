// Recurring Command: "tempr,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,_!"
// Immediate Command: "tempi,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,_!"
// Acknowledgement to Run: "tempa,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,_!"



#include <evolver_si.h>
#include <Tlc5940.h>
#include <PID_v1.h>

// Serial Event Variables
String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete

// Mux Control Pins
int s0 = 7, s1 = 8, s2 = 9, s3 = 10, SIG_PIN = 0;

// Serial Communication Variables
String comma = ",";
String end_mark = "end";
String address = "temp";
int num_vials = 16;
evolver_si in("temp", "_!", num_vials+1); // 17 CSV-inputs from RPI
boolean new_input = false;
int saved_inputs[] = {4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095};

// PID Settings
double Setpoint[] = {4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095};
double Output[16], Input[16];
int Kp = 6500;
int Ki = 20;
int Kd = 2;

PID pid1(&Input[0], &Output[0], &Setpoint[0], Kp, Ki, Kd, DIRECT);//650, 12, 2
PID pid2(&Input[1], &Output[1], &Setpoint[1], Kp, Ki, Kd, DIRECT);
PID pid3(&Input[2], &Output[2], &Setpoint[2], Kp, Ki, Kd, DIRECT);
PID pid4(&Input[3], &Output[3], &Setpoint[3], Kp, Ki, Kd, DIRECT);
PID pid5(&Input[4], &Output[4], &Setpoint[4], Kp, Ki, Kd, DIRECT);
PID pid6(&Input[5], &Output[5], &Setpoint[5], Kp, Ki, Kd, DIRECT);
PID pid7(&Input[6], &Output[6], &Setpoint[6], Kp, Ki, Kd, DIRECT);
PID pid8(&Input[7], &Output[7], &Setpoint[7], Kp, Ki, Kd, DIRECT);
PID pid9(&Input[8], &Output[8], &Setpoint[8], Kp, Ki, Kd, DIRECT);
PID pid10(&Input[9], &Output[9], &Setpoint[9], Kp, Ki, Kd, DIRECT);
PID pid11(&Input[10], &Output[10], &Setpoint[10], Kp, Ki, Kd, DIRECT);
PID pid12(&Input[11], &Output[11], &Setpoint[11], Kp, Ki, Kd, DIRECT);
PID pid13(&Input[12], &Output[12], &Setpoint[12], Kp, Ki, Kd, DIRECT);
PID pid14(&Input[13], &Output[13], &Setpoint[13], Kp, Ki, Kd, DIRECT);
PID pid15(&Input[14], &Output[14], &Setpoint[14], Kp, Ki, Kd, DIRECT);
PID pid16(&Input[15], &Output[15], &Setpoint[15], Kp, Ki, Kd, DIRECT);
PID *allPIDS[16] = {&pid1, &pid2, &pid3, &pid4, &pid5, &pid6, &pid7, &pid8, &pid9, &pid10, &pid11, &pid12, &pid13, &pid14, &pid15, &pid16};


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

  Tlc.init(LEFT_PWM,4095);
  // initialize serial:
  Serial1.begin(9600);
  SerialUSB.begin(9600);
  // reserve 2000 bytes for the inputString:
  inputString.reserve(2000);

  while (!Serial1);

  for (int i = 0; i < num_vials; i++) {
    allPIDS[i]->SetOutputLimits(0, 4095);
    allPIDS[i]->SetMode(AUTOMATIC);
  }
}

void loop() {
  serialEvent();
  if (stringComplete){
    SerialUSB.println(inputString);
    in.analyzeAndCheck(inputString);

    // Clear input string, avoid accumulation of previous messages
    inputString = "";

    if (in.addressFound) {
      if (in.input_array[0] == "i" || in.input_array[0] == "r") {
        
        SerialUSB.println("Saving Setpoints");
        for (int n = 1; n < num_vials+1; n++) {
          saved_inputs[n-1] = in.input_array[n].toInt();
        }
        
        SerialUSB.println("Responding with Data...");
        new_input = true;
        dataResponse();
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

    
    in.addressFound = false;
    stringComplete = false;
  }
  // Update PID every loop for better temp control
  read_MuxShield();
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


void dataResponse() {
  digitalWrite(12, HIGH);
  String outputString = address + "b,"; // b is a broadcast tag
  for (int i = 0; i < num_vials; i = i + 1) {
    outputString += String((int)Input[i]) + comma;
  }
  outputString += end_mark;
  delay(100); // important to make sure pin 12 flips appropriately
  SerialUSB.println(outputString);
  Serial1.print(outputString); // issues w/ println on Serial 1 being read into Raspberry Pi
  delay(100); // important to make sure pin 12 flips appropriately
  digitalWrite(12, LOW);
}

void update_values() {
  for (int i = 0; i < num_vials; i = i + 1) {
    Setpoint[i] = (double)saved_inputs[i];
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
    Input[m] = mux_readings[m];
  }

  for (int i = 0; i < num_vials; i = i + 1) {
    allPIDS[i]->Compute();
    int set_value = Output[i];
    Tlc.set(LEFT_PWM,i, set_value);
    //SerialUSB.print(Output[i]);
  }
  while(Tlc.update());
}

int readMux(int channel) {
  int controlPin[] = {s0, s1, s2, s3};

  int muxChannel[16][4] = {
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
  for (int i = 0; i < 4; i ++) {
    digitalWrite(controlPin[i], muxChannel[channel][i]);
  }

  //read the value at the SIG pin
  int val = analogRead(SIG_PIN);

  //return the value
  return val;
}
