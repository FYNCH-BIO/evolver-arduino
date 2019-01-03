#include <evolver_si.h>

String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete

evolver_si m("XR"," !",3); // Creating an instance of evolver_si

void setup() {
  // initialize serial:
  SerialUSB.begin(9600);
  // reserve 1000 bytes for the inputString:
  inputString.reserve(1000);
}

void loop() {
  if (SerialUSB.available()) {
    while (SerialUSB.available()) // While data is available
    {
      // Read from SerialUSB and add to the string:
      inputString += (char)SerialUSB.read();
    }

    SerialUSB.println(inputString);

    // Analyze and Check the input string to obtain commands
    m.analyzeAndCheck(inputString);

    // If successfully analyzed then do the following commands
    if(m.addressFound){

      for (int i = 0; i < m.num_vials; i++){
        SerialUSB.println("Values: " + m.input_array[i]);
      }

   }

    // clear the string and delete memory
    inputString = "";
    stringComplete = false;
    m.addressFound = false;
  }
}
