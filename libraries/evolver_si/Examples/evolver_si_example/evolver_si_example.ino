#include <evolver_si.h>

String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete

void setup() {
  // initialize serial:
  Serial.begin(9600);
  // reserve 1000 bytes for the inputString:
  inputString.reserve(1000);
}


evolver_si m("XR"," !",3); // Creating an instance of evolver_si
evolver_si n("re"," !",0);

void loop() {

  // print the string when a newline arrives and if complete
  if (stringComplete) {
    Serial.println(inputString);
    // Analyze and Check the input string to obtain commands
    m.analyzeAndCheck(inputString);
    n.analyzeAndCheck(inputString);

    // If successfully analyzed then do the following commands
    if(m.addressFound){
      Serial.println("FOUND!");
      for (int i = 0; i < m.num_vials; i++){
        Serial.println("Values: " + m.input_array[i]);
      }

   }
       // If successfully analyzed then do the following commands
    if(n.addressFound){
      Serial.println("FOUND!");
   }


    // clear the string and delete memory
    inputString = "";
    stringComplete = false;
    m.addressFound = false;
    n.addressFound = false;
  }
}

/*
  SerialEvent occurs whenever a new data comes in the
 hardware serial RX.  This routine is run between each
 time loop() runs, so using delay inside loop can delay
 response.  Multiple bytes of data may be available.
 */
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '!') {
      stringComplete = true;
    }
  }
}
