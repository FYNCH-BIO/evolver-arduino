/// INSTANT COMMAND: pumpi,2|4,10,--,--,--,--,--,--,--,--,--,--,--,--,--,--,3|4,15,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,_!
/// RECURRING COMMAND: pumpr,2|4,10,--,--,--,--,--,--,--,--,--,--,--,--,--,--,3|4,15,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,_!
/// TURN OFF ALL PUMPS WITH: pumpr,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,_!
/// ACKNOWLEDGMENT TO RUN: pumpa,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,_!

/// IPP USAGE: pumpi,<Hz>|<Pump number>|<IPP index>,...
/// To run 2 IPPs on addresses (0,1,2), and (3,4,5):
/// pumpi,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,10|1|1,10|1|2,10|1|3,5|2|1,5|2|2,5|2|3,--,--,--,--,--,--,--,--,--,--,_!
/// Then acknowledge like above.

/// To turn off IPP, a command with 0 as Hz to the desired pump number.
/// pumpi,0|1|1,0|2|1,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,_!

#include <evolver_si.h>
#include <Tlc5940.h>

const int numPumps = 48;
String inputString = "";
boolean stringComplete = false;
double timeToPump;
int pumpInterval;
int speedset[2] = {4095, 0}; // (fully off, max speed)

String address = "pump";
String comma = ",";
String end_mark = "end";
evolver_si in("pump", "_!", numPumps+1);// 49 CSV-inputs from RPI 
boolean newInput = false;
String savedInputs[48];


class Pump {
  boolean pumpRunning = false;
  int addr;
  int timeToPump = 0;
  int pumpInterval = 0;
  unsigned long previousMillis = 0;
  boolean off = false;

  public:
    Pump() {}

    // Sets address for each pump if applicable. 
    void init(int addrInit) {      
      addr = addrInit;
    }

    // Checks based on settings whether to start/stop pumping. 
    void update() {
      unsigned long currentMillis = millis();
      // Stop pump if pump is running and time expired.
      if (currentMillis - previousMillis > timeToPump && pumpRunning) {
        Tlc.set(LEFT_PWM, addr, speedset[0]);
        pumpRunning = false;
      }

      // Start the pump if repeats desired (chemostat, breaking up large pump event)
      if (currentMillis - previousMillis > pumpInterval && !pumpRunning && pumpInterval != 0) {
        previousMillis = currentMillis;
        pumpRunning = true;
        Tlc.set(LEFT_PWM, addr, speedset[1]);
      }
    }

    // Turns on the pump based on passed command
    void setPump(float timeToPumpSet, int pumpIntervalSet) {
      timeToPump = timeToPumpSet * 1000; // convert to millis
      pumpInterval = pumpIntervalSet * 1000; // conver to millis

      // Start pump and set timing
      previousMillis = millis();
      pumpRunning = true;
      Tlc.set(LEFT_PWM, addr, speedset[1]);
    }

    void turnOff() {
      Tlc.set(LEFT_PWM, addr, speedset[0]);
    }

    bool isNewChemostat(float newTimeToPump, int newPumpInterval){
      newTimeToPump = newTimeToPump * 1000; // convert to millis
      newPumpInterval = newPumpInterval * 1000; // conver to millis

      return (newTimeToPump == timeToPump && newPumpInterval == pumpInterval);
    }
};

class IPP {
  int solenoid1;
  int solenoid2;
  int solenoid3;
  int solenoidState = -1;
  unsigned long previousMillis;
  long holdTime = -1;

  public:
  IPP() {}
  IPP(int _solenoid1, int solenoid2, int solenoid3) {
    this->solenoid1 = solenoid1;
    this->solenoid2 = solenoid2;
    this->solenoid3 = solenoid3;
  }

  void update(long holdTime) {   
    SerialUSB.println("Updating holdtime"); 
    this->holdTime = holdTime;
    this->update();
    if (solenoidState == -1) {
      this->solenoidState = 1; 
    }    
  }

  void update() {
    unsigned long currentMillis = millis();
    if (holdTime == -1 && solenoid1) {
      Tlc.set(LEFT_PWM, solenoid1, speedset[0]);
      Tlc.set(LEFT_PWM, solenoid2, speedset[0]);
      Tlc.set(LEFT_PWM, solenoid3, speedset[0]);
    }
    else {
      boolean timeExpired = currentMillis - previousMillis >= holdTime;
      if (solenoidState == 1 && timeExpired) {
        solenoidState = 2;
        previousMillis = currentMillis;
        Tlc.set(LEFT_PWM, solenoid1, speedset[0]);
        Tlc.set(LEFT_PWM, solenoid2, speedset[1]);
      }
      else if (solenoidState == 2 && timeExpired) {
        solenoidState = 3;
        previousMillis = currentMillis;
        Tlc.set(LEFT_PWM, solenoid2, speedset[0]);
        Tlc.set(LEFT_PWM, solenoid3, speedset[1]);
      }
      else if (solenoidState == 3 && timeExpired) {
        solenoidState = 1;
        previousMillis = currentMillis;
        Tlc.set(LEFT_PWM, solenoid3, speedset[0]);
        Tlc.set(LEFT_PWM, solenoid1, speedset[1]);  
      }      
    }
  }

  void turnOff() {
    solenoidState = -1;
    Tlc.set(LEFT_PWM, solenoid1, speedset[0]);
    Tlc.set(LEFT_PWM, solenoid2, speedset[0]);
    Tlc.set(LEFT_PWM, solenoid3, speedset[0]);
    this->update();
  }

  void setAddress(int address, int solenoidNumber) {
    if (solenoidNumber == 1) { 
      solenoid1 = address;
    }
    else if (solenoidNumber == 2) {
      solenoid2 = address;  
    }
    else if (solenoidNumber == 3) {
      solenoid3 = address;  
    }    
  }
};

Pump pumps[numPumps];
IPP ippPumps[numPumps];

void setup() {
  pinMode(12, OUTPUT);
  digitalWrite(12, LOW);

  // initialize serial:
  Serial1.begin(9600);
  SerialUSB.begin(9600);
  // reserve 200 bytes for the inputString:
  inputString.reserve(2000);
  Tlc.init(LEFT_PWM, speedset[0]);

  //Initialize all the pumps
  // UNCOMMENT HERE FOR OLD PUMP SETTING (RevA)
  //for (int i = 0; i < numPumps; i++) {
  //  pumps[i].init(i);
  //}

  // UNCOMMENT HERE FOR NEW PUMP SETTING (RevB)

  for (int i = 0; i < 6; i++) { //6 pump racks
    for (int j = 0; j < 8; j++) { //8 pumps per racks
      int pumpIndx = 8*i + j;
      pumps[pumpIndx].init( 8*i + 7 -j);
    }
  }

  while (!Serial1);
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
  for (int n = 0; n < numPumps; n++) {
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


void loop() {
  // Read Serial Inputs and checks if it is the right addresses
  serialEvent();
  if (stringComplete) {
    SerialUSB.println(inputString);
    in.analyzeAndCheck(inputString);

    // Clear input string, avoid accumulation of previous messages
    inputString = "";

    if (in.addressFound) {
      if (in.input_array[0] == "i" || in.input_array[0] == "r") {
         
        SerialUSB.println("Saving Setpoints");
        for (int n = 1; n < numPumps+1; n++) {
          savedInputs[n-1] = in.input_array[n];
        }
        
        SerialUSB.println("Echoing New Pump Command");
        newInput = true;
        echoCommand();
        SerialUSB.println("Waiting for OK to execute...");
      }

 
      if (in.input_array[0] == "a" && newInput) {
        for (int i = 0; i < numPumps; i++) {
          if (savedInputs[i] != "--") {
            int splitIndx = savedInputs[i].indexOf("|");
            int ippSplitIndx = savedInputs[i].indexOf("|",splitIndx+1);
            if (splitIndx == -1) {
              timeToPump = savedInputs[i].toFloat();
              pumpInterval = 0;
            } 
            else if  (ippSplitIndx == -1) {
              timeToPump = savedInputs[i].substring(0, splitIndx).toFloat();
              pumpInterval = savedInputs[i].substring(splitIndx+1).toInt();
            } 
            else {
              int pumpNumber = savedInputs[i].substring(splitIndx+1, ippSplitIndx).toInt();
              int solenoidNumber = savedInputs[i].substring(ippSplitIndx+1).toInt();
              float hz = savedInputs[i].substring(0, splitIndx).toFloat();
              int holdTime;
              if (hz > 0) {
                holdTime = (1.0 / hz) * 1000;
                ippPumps[pumpNumber].setAddress(i, solenoidNumber);
                ippPumps[pumpNumber].update(holdTime);                
              }
              else {
                ippPumps[pumpNumber].turnOff();  
              }
            }

            // If chemostat command, verify that this is a new command
            if (pumpInterval != 0 &&
                pumps[i].isNewChemostat(timeToPump, pumpInterval)){
               SerialUSB.print("Unaltered chemostat: ");
               SerialUSB.println(i);               
            }
            else if (ippSplitIndx == -1) {
               SerialUSB.print("Pump: ");
               SerialUSB.println(i);
               pumps[i].setPump(timeToPump, pumpInterval);
            }
          }
        }
        
        SerialUSB.println("Command Executed!");
        newInput = false;
      }
        

    inputString = "";
    }
    // clear the string:
    stringComplete = false;
    in.addressFound = false;
  }

  // Update all the pumps
  for (int i = 0; i < numPumps; i++) {
      pumps[i].update();
      ippPumps[i].update();
  }

  // Update the PWM - do not call this more than one time per loop
  Tlc.update();

  // Clears strings if too long
  // Should be checked server-side to avoid malfunctioning
  if (inputString.length() > 2000){
    SerialUSB.println("Cleared Input String");
    inputString = "";
  }
}
