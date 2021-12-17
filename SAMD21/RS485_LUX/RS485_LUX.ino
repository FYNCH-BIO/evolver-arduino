#include <evolver_si.h>

String inputString = "";         // a String to hold incoming data
String input2String = "";
int stringComplete = false;  // whether the string is complete
int string2Complete = false;

int ch1Data = 0;
int ch2Data = 0;

int const numVials = 16;
int muxReadings[numVials]; // The size Assumes number of vials
int activeVial = 2;
int savedTimesAveraged = 5;
int timesAveraged = 5; // Number of times to average values from one vial before moving on
int tempControlData = 0;
int tempSignalData = 0;
int dataCount = 0;
unsigned long lastTimeMeasurementsStarted;
boolean newLuxInput = false;
int luxVials[] = {4,5,6,7,12,13,14,15};
const int luxLen = sizeof(luxVials) / sizeof(luxVials[0]); //how many lux vials there are
int vialCount = 0; // for iterating through only the lux vials
boolean first = true;

int s0 = 7, s1 = 8, s2 = 9, s3 = A0;
int controlPins[] = {s0, s1, s2, s3};

int output[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

String comma = ",";
String endMark = "end";
String luxAddress = "lux";

evolver_si in("lux", "_!", 2);

//int muxChannels[16][4] = {
//  {0, 0, 0, 0}, //channel 0; Vial 0
//  {1, 1, 0, 0}, //channel 3; Vial 1
//  {1, 0, 0, 0}, //channel 1; Vial 2
//  {0, 1, 0, 0}, //channel 2; Vial 3
//  {0, 0, 1, 0}, //channel 4; Vial 4
//  {1, 1, 1, 0}, //channel 7; Vial 5
//  {1, 0, 1, 0}, //channel 5; Vial 6
//  {0, 1, 1, 0}, //channel 6; Vial 7
//  {0, 0, 0, 1}, //channel 8; Vial 8
//  {1, 1, 0, 1}, //channel 11; Vial 9
//  {1, 0, 0, 1}, //channel 9; Vial 10
//  {0, 1, 0, 1}, //channel 10; Vial 11
//  {0, 0, 1, 1}, //channel 12; Vial 12
//  {1, 1, 1, 1}, //channel 15; Vial 13
//  {1, 0, 1, 1}, //channel 13; Vial 14
//  {0, 1, 1, 1}  //channel 14; Vial 15
//};
//

int muxChannels[16][4] = {
  {0, 0, 0, 0}, //channel 0; Vial 0
  {1, 0, 0, 0}, //channel 3; Vial 1
  {0, 1, 0, 0}, //channel 1; Vial 2
  {1, 1, 0, 0}, //channel 2; Vial 3
  {0, 0, 1, 0}, //channel 4; Vial 4
  {1, 0, 1, 0}, //channel 7; Vial 5
  {0, 1, 1, 0}, //channel 5; Vial 6
  {1, 1, 1, 0}, //channel 6; Vial 7
  {0, 0, 0, 1}, //channel 8; Vial 8
  {1, 0, 0, 1}, //channel 11; Vial 9
  {0, 1, 0, 1}, //channel 9; Vial 10
  {1, 1, 0, 1}, //channel 10; Vial 11
  {0, 0, 1, 1}, //channel 12; Vial 12
  {1, 0, 1, 1}, //channel 15; Vial 13
  {0, 1, 1, 1}, //channel 13; Vial 14
  {1, 1, 1, 1}  //channel 14; Vial 15
};

Uart Serial2 (&sercom1, 11, 10, SERCOM_RX_PAD_0, UART_TX_PAD_2);
 
void SERCOM1_Handler()
{
  Serial2.IrqHandler();
}

void setup() {
  // initialize serial:
  Serial1.begin(9600);
  Serial2.begin(9600);
  SerialUSB.begin(9600);
  // reserve 200 bytes for the inputString:
  inputString.reserve(200);
  input2String.reserve(200);

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
  //lastVial = millis();
}

void loop() {
  // print the string when a newline arrives:
  serialEvent();
  serial2Event();
  
      // if it takes too long, move on to next vial. Problem with device or not connected.
  if ((millis() - lastTimeMeasurementsStarted) > 4000) {
    activeVial = luxVials[vialCount];
    vialCount += 1;
    if (vialCount >= luxLen) {
      vialCount = 0;
    }
    setMux(activeVial);
    SerialUSB.print("slow measurements - Vial: ");
    SerialUSB.print(activeVial);
    SerialUSB.print("| Time: ");
    SerialUSB.print(millis());
    SerialUSB.print("; Last Time: ");
    SerialUSB.println(lastTimeMeasurementsStarted);
    lastTimeMeasurementsStarted = millis();
  }

  if (string2Complete) {
    if (!first) { // wait until a full message is sent from ATTiny before recording
      int tabIndex = input2String.indexOf("|");
      ch1Data = input2String.substring(0, tabIndex).toInt();
      ch2Data = input2String.substring(tabIndex + 1, input2String.length() - 1).toInt();
      
      tempControlData += ch1Data; // store the data for averaging
      tempSignalData += ch2Data;
      dataCount++;

      if (dataCount >= timesAveraged) {
        // average the values for this vial timesAveraged times
        output[activeVial] = tempControlData / timesAveraged;
        output[activeVial + numVials] = tempSignalData / timesAveraged;
        SerialUSB.print("Average Channel1: ");
        SerialUSB.println(output[activeVial]);
        SerialUSB.print("Average Channel2: ");
        SerialUSB.println(output[activeVial + numVials]);
        tempControlData = 0;
        tempSignalData = 0;
        
        // iterating through just the lux vials
        activeVial = luxVials[vialCount];
        vialCount += 1;
        dataCount = 0;
        if (vialCount >= luxLen) {
          vialCount = 0;
        }
        setMux(activeVial);
        first = true;
      }
  
      input2String = "";
      string2Complete = false;
    }
    else {
      SerialUSB.println();
      SerialUSB.print("Vial: ");
      SerialUSB.println(activeVial);
//      SerialUSB.print("Skipped ATTiny string: ");
//      SerialUSB.println(input2String);
      first = false;
      input2String = "";
      string2Complete = false;
      lastTimeMeasurementsStarted = millis();
    }
  }

  if (stringComplete) {    
    in.analyzeAndCheck(inputString);

    if (in.addressFound) {
      SerialUSB.println(inputString);
      if (in.input_array[0] == "i" || in.input_array[0] == "r") {
        SerialUSB.println("Saving Averaging Setting");
        savedTimesAveraged = in.input_array[1].toInt();
        SerialUSB.println("Echoing new lux command");
        newLuxInput = true;
        dataResponse();

        SerialUSB.println("Waiting for OK to execute...");        
      }

      if (in.input_array[0] == "a" && newLuxInput) {
        timesAveraged = savedTimesAveraged;
        SerialUSB.println("Lux command executed!");
        newLuxInput = false;
      }
    }   

    in.addressFound = false;
    
    inputString = "";
    stringComplete = false;
  }
}

void serialEvent() {
  char inChar;
  while (Serial1.available()) {
    // get the new byte:
    inChar = (char)Serial1.read();    
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    if (inChar == '!') {
      stringComplete = true;
      break;
    }
  }
}

void serial2Event() {
  char inChar;
  while (Serial2.available()) {
    inChar = (char)Serial2.read();
    input2String += inChar;    
     if (inChar == '!') {      
      string2Complete = true;      
      break;
     }
  }        
}

void dataResponse() {  
  digitalWrite(12, HIGH);
  String outputString = luxAddress + "b,"; // b is a broadcast tag
  for (int n = 0; n < numVials * 2; n++) {
    outputString += output[n];
    outputString += comma;  
  }
  outputString += endMark;

  delay(100); // important to make sure pin 12 flips appropriately

  SerialUSB.println(outputString);
  Serial1.print(outputString); // issues w/ println on Serial 1 being read into Raspberry Pi  

  delay(100); // important to make sure pin 12 flips appropriately  
  digitalWrite(12, LOW);  
}

void setMux(int channel) {
  // loop through the four signal pins
  for (int i = 0; i < 4; i++) {
    digitalWrite(controlPins[i], muxChannels[channel][i]);    
  }
}
