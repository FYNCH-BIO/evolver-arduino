///// eVOLVER FLUIDIC CONTROL VERSION 6
///// Written by Zachary Heins
///// This version of the code has built in turbidostat and chemostat functionalities - generalized pump logic

//// Command structure: "st<MODE><time to pump>,<time to pump efflux extra>,<delay interval>,<times to repeat>,<run efflux simultaneously>,<vials binary>,0,0,0,0,0,0,0,0,0,0,0,0, !"
//// Accepted values for <MODE>: "p" (for pump), "o" (for off).
//// Sample Commands sent over RS485 (Serial1):
//// Turn on pump 3 for 5s every 10s indefinitely (by passing negative value to <times to repeat>), with 2s of efflux extra after: "stp,5,2,10,-1,1,100,0,0,0,0,0,0,0,0,0,0,0, !"
//// Turn on pump 18 as efflux for 10s one time: "stc,p,10,0,0,0,0,100000000000000000,0,0,0,0,0,0,0,0,0,0,0, !"
//// Force all pumps to stop: "stc,o,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, !"
//// Individual or subsets of pumps can be stopped by sending a command to them with a "0" for the pump time.

//// NOTES:
//// <times to repeat> can be set to 0 or to 1 to pump only 1 time. Any positive value will pump that many times. Negative values will set it to pump until a new command sent.
//// There are currently no checks that the values sent to set pump actually make sense, ie intervals shorter than the pump time/efflux time.


#include <evolver_si.h>
#include <Tlc5940.h>

int expected_inputs = 19;
String address = "st";
String comma = ",";
String end_mark = "end";
evolver_si in("st", " !", expected_inputs);


const int numPumps = 32;
String fluid_mode = "";
String inputString = "";
String binaryString = "";
boolean stringComplete = false;
double timeToPump;
double timeToEffluxPump;
int pumpInterval;
int numberTimesToPump;
int runEfflux;
int speedset[2] = {4095, 0}; // (fully off, max speed)

class Pump {
  boolean pumpRunning = false;
  boolean effluxRunning = false;
  int addr;
  int effluxAddr;
  int timeToPump = 0;
  int timeToEffluxPump = 0;
  int pumpInterval = 0;
  int numberTimesToPump = 0;
  unsigned long previousMillis = 0;
  int runEfflux = 0;
  boolean off = false;

  public:
    Pump() {}

    // Sets address for each pump + efflux pump if applicable. 
    void init(int addrInit, int effluxAddrInit = NULL) {      
      addr = addrInit;
      if (effluxAddrInit) {
        effluxAddr = effluxAddrInit;
      }
      else {        
        effluxAddr = addr + 16;
      }
    }

    // Checks based on settings whether to start/stop pumping. 
    void update() {
      unsigned long currentMillis = millis();
      // Stop pump if pump is running and time expired.
      if (currentMillis - previousMillis > timeToPump && pumpRunning) {
        Tlc.set(LEFT_PWM, addr, speedset[0]);
        pumpRunning = false;
      }

      // Stop the efflux pump if running and time expired
      if (currentMillis - previousMillis > timeToPump + timeToEffluxPump && effluxRunning) {
        Tlc.set(LEFT_PWM, effluxAddr, speedset[0]);
        effluxRunning = false;
      }

      // Start the pump if repeats desired (chemostat, breaking up large pump event)
      if (numberTimesToPump != 0 && currentMillis - previousMillis > pumpInterval && !pumpRunning) {
        // No need to subtract if already < 0
        if (numberTimesToPump > 0) {
          numberTimesToPump = numberTimesToPump - 1; 
        }        
        previousMillis = currentMillis;
        pumpRunning = true;
        Tlc.set(LEFT_PWM, addr, speedset[1]);
        if (runEfflux) {
          effluxRunning = true;
          Tlc.set(LEFT_PWM, effluxAddr, speedset[1]);
        }
      }
    }

    // Turns on the pump based on passed command
    void setPump(float timeToPumpSet, float timeToEffluxPumpSet, int pumpIntervalSet, int numberTimesToPumpSet, int runEffluxSet) {
      timeToPump = timeToPumpSet * 1000; // convert to millis
      timeToEffluxPump = timeToEffluxPumpSet * 1000; //convert to millis
      pumpInterval = pumpIntervalSet * 1000; // conver to millis
      numberTimesToPump = numberTimesToPumpSet;

      // If this field is being used. Can be 0 or 1 for 1 pump. Negative means pump continously.
      if (numberTimesToPump != 0) {
        numberTimesToPump = numberTimesToPump - 1;
      }
      runEfflux = runEffluxSet;

      // Start pump and set timing
      previousMillis = millis();
      pumpRunning = true;
      Tlc.set(LEFT_PWM, addr, speedset[1]);
      if (runEfflux) {
        effluxRunning = true;
        Tlc.set(LEFT_PWM, effluxAddr, speedset[1]);
      }
    }

    void turnOff() {
      numberTimesToPump = 0;
      Tlc.set(LEFT_PWM, addr, speedset[0]);
      if (runEfflux) {
        Tlc.set(LEFT_PWM, effluxAddr, speedset[0]);
      }
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

  // Initialize all the pumps
  for (int i = 0; i < numPumps; i++) {
    pumps[i].init(i);
  }

  while (!Serial1);
}

void serialEvent() {
  while (SerialUSB.available()) {
    char inChar = (char)SerialUSB.read();
    inputString += inChar;
    if (inChar == '!') {
      stringComplete = true;
    }
  }
}

void echoCommand() {
  digitalWrite(12, HIGH);
  
  String outputString = address + "e,";
  for (int n = 1; n < expected_inputs; n++) {
    outputString += in.input_array[n];
    outputString += comma;
  }
  outputString += end_mark;
  SerialUSB.println(outputString);
  Serial1.println(outputString);
  
  digitalWrite(12, LOW);
}


void loop() {
  // Read Serial Inputs and checks if it is the right addresses
  serialEvent();
  if (stringComplete) {
    SerialUSB.println(inputString);
    in.analyzeAndCheck(inputString);

    if (in.addressFound) {
      SerialUSB.println("Input Address Found");
            
      if (in.input_array[0] == "c") {
        SerialUSB.println("Echoing command!");
        echoCommand();
      }

      fluid_mode = in.input_array[1]; // Obtains Fluid Mode ("p" or "o")
      
      if (fluid_mode == "p") {
        timeToPump = in.input_array[2].toDouble();        
        timeToEffluxPump = in.input_array[3].toDouble();
        pumpInterval = in.input_array[4].toInt();        
        numberTimesToPump = in.input_array[5].toInt();        
        runEfflux = in.input_array[6].toInt();
        binaryString = in.input_array[7];

        // Parses binaryString and sets the pumps accordingly. Applies the command to every pump seen with '1' in the string, with pump 0 being the right-most value in the string
        // For example, a binaryString of 101001 turns on pumps 0, 3, and 5
        for (int i = 0; i <= binaryString.length(); i++) {
          if (binaryString[binaryString.length() - i - 1] == '1') {
            pumps[i].setPump(timeToPump, timeToEffluxPump, pumpInterval, numberTimesToPump, runEfflux);
          }
        }
      }

      inputString = "";
    }
    // clear the string:
    stringComplete = false;
    in.addressFound = false;
  }

  // Update all the pumps
  for (int i = 0; i < numPumps; i++) {
    if (fluid_mode == "o") {
      pumps[i].turnOff();
    }
    else {
      pumps[i].update(); 
    }
  }

  // Update the PWM - do not call this more than one time per loop
  Tlc.update();

  //Clears strings if too long
  if (inputString.length() >900){
    SerialUSB.println("Cleared Input String");
    inputString = "";
  }
}
