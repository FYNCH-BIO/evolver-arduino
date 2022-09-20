#include <evolver_si.h>
#include <PID_v1.h>
#include <SAMD21turboPWM.h>

// String Input
String inputString = "";
boolean stringComplete = false;
boolean serialAvailable = true;

int num_vials = 2;
int active_vial = 0;
int PDtimes_averaged = 500;
int pdOutput[] = {60000, 60000};
int pdInput[] = {255, 255};

int pd_pin[] = {A2, A3};
int temp_pin[] = {A0, A1};

// General Serial Communication
String comma = ",";
String end_mark = "end";

// Photodiode Serial Communication
int expectedPDinputs = 2;
String photodiode_address = "od_90";
evolver_si pd("od_90", "_!", expectedPDinputs);
boolean new_PDinput = false;
int saved_PD_averaged = 1000;

//LED Settings
String led_address = "od_led";
evolver_si led("od_led", "_!", num_vials + 1);
boolean new_LEDinput = false;
int saved_LEDinputs[] = {255, 255};

// TEMP
double tempSetpoint[] = {60000, 60000};
double tempOutput[2], tempInput[2];
int tempOutputPin[] = {2, 3};
String temp_address = "temp";
boolean temp_new_input = false;
evolver_si temp("temp", "_!", num_vials + 1);
int temp_saved_inputs[] = {255, 255};

int Kp = 6500;
int Ki = 20;
int Kd = 2;

PID pid1(&tempInput[0], &tempOutput[0], &tempSetpoint[0], Kp, Ki, Kd, DIRECT);
PID pid2(&tempInput[1], &tempOutput[1], &tempSetpoint[1], Kp, Ki, Kd, DIRECT);
PID *allTempPIDS[16] = {&pid1, &pid2};

// Stir
TurboPWM pwm;
String stir_address = "stir";
evolver_si stir("stir", "_!", num_vials + 1);
boolean stir_new_input = false;
int stir_saved_inputs[] = {0, 0};
int stirInput[] = {1000, 1000};
int stirOutputPin[] = {11, 13};

// Pumps
String pumpAddress = "pump";
const int numPumps = 6;
double timeToPump;
int pumpInterval;
int speedset[2] {0, 255};
evolver_si pump("pump", "_!", numPumps+1);
String pumpSavedInputs[6];
boolean pump_new_input = false;
int pumpOutputPin[] = {6, 7, 8, 9, 10, 12};
byte ipp = 0;

class Pump {
  boolean pumpRunning = false;
  int addr; int timeToPump = 0;
  int pumpInterval = 0;
  unsigned long previousMillis = 0;
  boolean off = false;

  public:
    Pump() {}
    void init(int addrInit) {
      addr = addrInit;
      analogWrite(pumpOutputPin[addr], speedset[0]);
    }

    void update() {
      unsigned long currentMillis = millis();
      if (currentMillis - previousMillis > timeToPump && pumpRunning) {
        analogWrite(pumpOutputPin[addr], speedset[0]);
        pumpRunning = false; 
      }

      if (currentMillis - previousMillis > pumpInterval && !pumpRunning && pumpInterval != 0) {
        previousMillis = currentMillis;
        pumpRunning = true;
        analogWrite(pumpOutputPin[addr], speedset[1]);  
      }
    }

    void setPump(float timeToPumpSet, int pumpIntervalSet) {
      timeToPump = timeToPumpSet * 1000;
      pumpInterval = pumpIntervalSet * 1000;

      previousMillis = millis();
      pumpRunning = true;
      analogWrite(pumpOutputPin[addr], speedset[1]);
    }

    void turnOff() {
      analogWrite(pumpOutputPin[addr], speedset[0]);
    }

    bool isNewChemostat(float newTimeToPump, int newPumpInterval) {
      newTimeToPump = newTimeToPump * 1000;
      newPumpInterval = newPumpInterval * 1000;

      return (newTimeToPump == timeToPump && newPumpInterval == pumpInterval);
    }
};

Pump pumps[numPumps];

void setup() {
  analogReadResolution(16);
  SerialUSB.begin(9600);
  // reserve 1000 bytes for the inputString
  inputString.reserve(1000);

  // Set up TurboPWM for stir
  pwm.setClockDivider(255, false);
  pwm.timer(2,256, 35000, true);
  pwm.analogWrite(11, 100);
  pwm.analogWrite(13, 100);

  for (int i = 0; i < numPumps; i++) {
    pumps[i].init(i);  
  }

  while(!SerialUSB);

  for (int i = 0; i < num_vials; i++) {
    allTempPIDS[i]->SetOutputLimits(0,255);
    allTempPIDS[i]->SetMode(AUTOMATIC);
  }
    
}

void loop() {
  readTemp();
  readPD();
  serialEvent();
  if (stringComplete) {
    pd.analyzeAndCheck(inputString);
    led.analyzeAndCheck(inputString);
    stir.analyzeAndCheck(inputString);
    temp.analyzeAndCheck(inputString);
    pump.analyzeAndCheck(inputString);

    // Clear input string, avoid accumulation of previous message
    inputString = "";

    // Photodiode logic
    if (pd.addressFound) {
      if (pd.input_array[0] == "i" || pd.input_array[0] == "r") {
        saved_PD_averaged = pd.input_array[1].toInt();
        new_PDinput = true;
        
        String outputString = photodiode_address + "b,";
        for (int n = 0; n < num_vials; n++) {
            outputString += pdOutput[n];
            outputString += comma;
        }  
        outputString += end_mark;
        SerialUSB.println(outputString);

      }
      if (pd.input_array[0] == "a" && new_PDinput) {
        PDtimes_averaged = saved_PD_averaged;
        new_PDinput = false;
      }      
    }

    // LED Logic
    if (led.addressFound) {
      if (led.input_array[0] == "i" || led.input_array[0] == "r") {
        for (int n = 1; n < num_vials+1; n++) {
          saved_LEDinputs[0] = led.input_array[1].toInt();
          saved_LEDinputs[0] = led.input_array[1].toInt();
          new_LEDinput = true;
        String outputString = led_address + "e,";
        for (int n = 1; n < num_vials + 1; n++) {
          outputString += led.input_array[n];
          outputString += comma;
        }
        outputString += end_mark;
        SerialUSB.println(outputString);
        
        }
        if (led.input_array[0] == "a" && new_LEDinput) {        
          analogWrite(4, saved_LEDinputs[0]);
          analogWrite(5, saved_LEDinputs[1]);
          new_LEDinput = false;
        }
        led.addressFound = false;
        inputString = "";
      }
      if (inputString.length() > 20000) {
        inputString = "";  
      }
      led.addressFound = false;
    }

    // Temp logic
    if (temp.addressFound) {
      if (temp.input_array[0] == "i" || temp.input_array[0] == "r") {
        for (int i = 1; i < num_vials + 1; i++) {
          temp_saved_inputs[i-1] = temp.input_array[i].toInt();  
        }
        temp_new_input = true;
        
        String outputString = temp_address + "b,";
        for (int n = 0; n < num_vials; n++) {
            outputString += String((int)tempInput[n]);
            outputString += comma;
        }  
        outputString += end_mark;
        SerialUSB.println(outputString);

      }
      if (temp.input_array[0] == "a" && temp_new_input) {
        for (int i = 0; i < num_vials; i++) {
          tempSetpoint[i] = (double)temp_saved_inputs[i];        
        }
        temp_new_input = false;
      }
      temp.addressFound = false;
    }

    // Stir logic
    if (stir.addressFound) {     
      if(stir.input_array[0] == "i" || stir.input_array[0] == "r") {
        for (int i = 1; i < num_vials+1; i++) {
          stir_saved_inputs[i-1] = stir.input_array[i].toInt();
        }
        String outputString = stir_address + "e,";
        for (int n = 1; n < num_vials + 1; n++) {
          outputString += stir.input_array[n];
          outputString += comma;
        }
        outputString += end_mark;
        SerialUSB.println(outputString);
        stir_new_input = true;
      }

      if (stir.input_array[0] == "a" && stir_new_input) {
        stir_new_input = false;
        for (int i = 0; i < num_vials; i++) {
          if ((int)stir_saved_inputs[i] > 0) {           
            pwm.analogWrite(stirOutputPin[i], 500);
            delay(50);
          }       
          pwm.analogWrite(stirOutputPin[i], (int)stir_saved_inputs[i] * 10);
        }
      }
      stir.addressFound = false;
    }

    //Pump logic
    if (pump.addressFound) {
      if (pump.input_array[0] == "i" || pump.input_array[0] == "r") {
        for (int i = 1; i < numPumps+1; i++) {
          pumpSavedInputs[i-1] = pump.input_array[i];  
        }
        pump_new_input = true;

        String outputString = pumpAddress + "e,";
        for (int n = 1; n < numPumps + 1; n++) {
          outputString += pump.input_array[n];
          outputString += comma;
        }
        outputString += end_mark;
        SerialUSB.println(outputString);
    
      }
      if (pump.input_array[0] == "a" && pump_new_input) {
        for (int i = 0; i < numPumps; i++) {
          if (pumpSavedInputs[i] != "--") {
            int splitIndx = pumpSavedInputs[i].indexOf("|");
            int ippSplitIndx = pumpSavedInputs[i].indexOf("|", splitIndx+1);
            if (splitIndx == -1) {
              timeToPump = pumpSavedInputs[i].toFloat();
              pumpInterval = 0;
            }
            else if (ippSplitIndx == -1) {
              timeToPump = pumpSavedInputs[i].substring(0, splitIndx).toFloat();
              pumpInterval = pumpSavedInputs[i].substring(splitIndx+1).toInt();
            }
            else {
              // IPP logic not supported!
              ipp++;
            }
  
            if (pumpInterval != 0 && pumps[i].isNewChemostat(timeToPump, pumpInterval)) {
              // unaltered chemostat
              SerialUSB.print("Unaltered chemostat: ");
              SerialUSB.println(i);
            }
            else if (ippSplitIndx == -1) {
              pumps[i].setPump(timeToPump, pumpInterval);  
            }
          }
        }
        pump_new_input = false;
      }      
    }
  }
  stringComplete = false;
  inputString = "";
  for (int i = 0; i < numPumps; i++) {
    pumps[i].update();  
  }
}

void serialEvent() {
  while (SerialUSB.available()) {
    char inChar = (char)SerialUSB.read();
    inputString += inChar;
    if (inChar == '!') {
      stringComplete = true;
      break;
    }
  }
}

void dataResponse(String addr, int data[]) {
  String outputString = addr + "b,";
  for (int n = 0; n < num_vials; n++) {
      outputString += data[n];
      outputString += comma;
  }  
  outputString += end_mark;
  SerialUSB.println(outputString);
}

void dataResponse(String addr, double data[]) {
  int data_int[2];
  for (int i = 0; i < num_vials; i++) {
    data_int[i] = data[i];
  }
  dataResponse(addr, data_int);
}

void readPD() {
  unsigned long total = 0;
  for (int i=0; i < PDtimes_averaged; i++) {
    total = total + analogRead(pd_pin[active_vial]);
    serialEvent();
    if (stringComplete) {
      break;
    }
  }
  if (!stringComplete) {
    pdOutput[active_vial] = total / PDtimes_averaged;
    if (active_vial == 1) {
      active_vial = 0;
    }
    else {
      active_vial++;  
    }
  }
}

void readTemp() {
  unsigned long total[num_vials];
  int readings[num_vials];
  int times_avg = 3;

  memset(total, 0 , sizeof(total));
  for (int i = 0; i < times_avg; i++) {
    for (int j = 0; j < num_vials; j++) {
      total[j] = total[j] + analogRead(temp_pin[j]);  
    }
  }

  for(int i = 0; i < num_vials; i++) {
    readings[i] = total[i] / times_avg;
    tempInput[i] = readings[i];    
  }

  for (int i = 0; i < num_vials; i++) {
    allTempPIDS[i]->Compute();
    int set_value = tempOutput[i];
    analogWrite(tempOutputPin[i], (255 - set_value));
  }
}
