/// INSTANT COMMAND: pumpi,2|4,10,--,--,--,--,--,--,--,--,--,--,--,--,--,--,3|4,15,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,_!
/// RECURRING COMMAND: pumpr,2|4,10,--,--,--,--,--,--,--,--,--,--,--,--,--,--,3|4,15,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,_!
/// TURN OFF ALL PUMPS WITH: pumpr,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,_!
/// ACKNOWLEDGMENT TO RUN: pumpa,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,_!


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
boolean new_input = false;
String saved_inputs[48];


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

Pump pumps[numPumps];

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
  for (int i = 0; i < numPumps; i++) {
    pumps[i].init(i);
  }

  // UNCOMMENT HERE FOR NEW PUMP SETTING (RevB)

//  for (int i = 0; i < 6; i++) { //6 pump racks
//    for (int j = 0; j < 8; j++) { //8 pumps per racks
//      int pump_indx = 8*i + j;
//      pumps[pump_indx].init( 8*i + 7 -j);
//    }
//  }

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
  
  delay(50); // important to make sure pin 12 flips appropriately
  SerialUSB.println(outputString);
  Serial1.print(outputString); // issues w/ println on Serial 1 being read into Raspberry Pi
  delay(50); // important to make sure pin 12 flips appropriately
  
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
          saved_inputs[n-1] = in.input_array[n];
        }
        
        SerialUSB.println("Echoing New Pump Command");
        new_input = true;
        echoCommand();
        SerialUSB.println("Waiting for OK to execute...");
      }

 
      if (in.input_array[0] == "a" && new_input) {
        for (int i = 0; i < numPumps; i++) {
          if (saved_inputs[i] != "--"){
            int split_indx = saved_inputs[i].indexOf("|");
            if (split_indx == -1){
              timeToPump = saved_inputs[i].toDouble();
              pumpInterval = 0;
              
            } else {
              timeToPump = saved_inputs[i].substring(0, split_indx).toDouble();
              pumpInterval = saved_inputs[i].substring(split_indx+1).toInt();
            }

            // If chemostat command, verify that this is a new command
            if (pumpInterval != 0 &&
                pumps[i].isNewChemostat(timeToPump, pumpInterval)){
               SerialUSB.print("Unaltered chemostat: ");
               SerialUSB.println(i);
            }
            else {
               SerialUSB.print("Pump: ");
               SerialUSB.println(i);
               pumps[i].setPump(timeToPump, pumpInterval);
            }
          }
        }
        
        SerialUSB.println("Command Executed!");
        new_input = false;
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
