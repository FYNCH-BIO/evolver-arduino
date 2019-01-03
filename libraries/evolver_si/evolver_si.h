/*

Module Header That Has Common Functions Used Across eVOLVER Experiments

*/

#ifndef evolver_si_h
#define evolver_si_h
#include "Arduino.h"

class evolver_si {

	public:
		evolver_si(char* start_Address, char* end_Address, int num_of_vials);
		~evolver_si(void);
		void analyzeAndCheck(String inputString);
		boolean addressFound;
		String input_array[32];
		int num_vials;

	private:
		char* _start_Address;
		char* _end_Address;

};


#endif
