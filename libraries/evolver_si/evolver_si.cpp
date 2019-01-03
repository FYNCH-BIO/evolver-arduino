#include "Arduino.h"
#include "evolver_si.h"


evolver_si::evolver_si(char* start_Address, char* end_Address, int num_of_vials) {

	_start_Address = start_Address; // tag used to identify a message part of the beginning of a string
	_end_Address = end_Address; // tag in the end used to identify the end of a string
	addressFound = false;
	num_vials = num_of_vials;

}

evolver_si::~evolver_si(){

	delete[] input_array;

}

void evolver_si::analyzeAndCheck(String inputString){
  // Debugging help
  // SerialUSB.println("Starting!!!");
  // SerialUSB.println(inputString);
  // SerialUSB.println(_start_Address);
  // SerialUSB.println(_end_Address);
  // SerialUSB.println(addressFound);
  // SerialUSB.println(num_vials);
  // SerialUSB.println(sizeof(_start_Address));
  // SerialUSB.println(sizeof(_end_Address));

  //
	// // // Finds and prints the indx of the starting address, ending address, and empty address
	int start_indx = inputString.indexOf(_start_Address);
	int end_indx = inputString.indexOf(_end_Address);
	if (start_indx != -1 && end_indx != -1){

		// Figure out
		inputString = inputString.substring(start_indx,end_indx+sizeof(_end_Address));
	 	start_indx = inputString.indexOf(_start_Address);
	 	end_indx = inputString.indexOf(_end_Address);

		//Serial.println(inputString);

		int num_commands = 0;int j = 0;
		String temp_string = "";

		// Counting the number of commands in the string
		for (int h = 0; h < inputString.length(); h++){
			if(inputString[h] == ','){
			num_commands++;
			}
		}

		// If the number of commands equals the number of vials
		if(num_commands == num_vials){
			// Look through string between the two tags

			for(int i = (start_indx + 2); i < end_indx; i++) {
				// Once Comma is reached convert the string to int and store into output input_array
				if (inputString[i] == ',') {
					input_array[j] = temp_string;
					j++;
					temp_string = "";

				} else {
					// Combine the elements between before the comma
					temp_string += inputString[i];
				}
			}
			addressFound = true;// Gives Condition To Do User Requests
		}
	}
}
