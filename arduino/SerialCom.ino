//----------------- SERIAL COMMUNICATION FUNCTIONS 

// Commands used to retrieve data from serial
enum SERIAL_TYPE {
	RAW,
	FILTER,
	TARGET
};

// add communication timeout
void sendSerialOutput( float measure[], float target[], uint8_t output_flag ) {
	for( uint8_t ch = 0; ch < N_ENABLED; ch += 1 ) {
		
		if( output_flag == FILTER) {				// filtered measure
			Serial.print( filter[ch].value() ); 
		
		} else if ( output_flag == RAW ) {			// raw measure
			Serial.print( measure[ch] );
		
		} else if ( output_flag == TARGET ) {		// target value
			Serial.print( target[ch] );
		}
		
		if( (ch+1) < N_ENABLED ) {					// do not print if last entry
			Serial.print(","); 	
		} // add delimited data for seperate blocks of real time data? -> mixed and raw
	}
	Serial.println();	// send data 
}

//-----

constexpr int INPUTS_MAX = 5; // <command> <param1> <param2> <param3> <param4> //target has max 4 inputs


	// parse string of numbers: num1, num2, num3 ...
uint8_t parseNumbers( char buffer[], float num_arr[] ) { 
	// Split string
	char* pointer;
	const char delim[] = ",";			// INPUT delimiter is string | IMPORTANT
	pointer = strtok(buffer, delim); 
	
	uint8_t index = 0;
	while( (pointer != nullptr) && (index < INPUTS_MAX) ) {
		// attempt conversion to double/float
		char * end_ptr;
		float number = strtod(pointer, &end_ptr); 
		
		if (*end_ptr == '\0') { 		// store if valid conversion
			num_arr[index] = number;
		} else {
			index = 0; 					// halt if string is not just delimited numbers
			break;
		}
		index += 1; // update index based on commas (fixed array position)
		
		// last call
		pointer = strtok(nullptr, delim);
	}
	return index;
}


//-----

enum FUNCTION {
	// getters
	GET_FILTER,			// 0
	GET_RAW,			// 1
	GET_TARGET,			// 2
	
		// PID
	GET_PID_CH0,		// 3
	GET_PID_CH1,		// 4
	GET_PID_CH2,		// 5
	GET_PID_CH3,		// 6
	
	GET_IN_LIMIT_CH0,	// 7
	GET_IN_LIMIT_CH1,	// 8
	GET_IN_LIMIT_CH2,	// 9
	GET_IN_LIMIT_CH3,	// 10
	
	GET_OUT_LIMIT_CH0,	// 11
	GET_OUT_LIMIT_CH1,	// 12
	GET_OUT_LIMIT_CH2,	// 13
	GET_OUT_LIMIT_CH3,	// 14
	
		// filters
	GET_AB_FILTER_CH0,	// 15
	GET_AB_FILTER_CH1,	// 16
	GET_AB_FILTER_CH2,	// 17
	GET_AB_FILTER_CH3,	// 18
	
	GET_K_FILTER_CH0,	// 19
	GET_K_FILTER_CH1,	// 20
	GET_K_FILTER_CH2,	// 21
	GET_K_FILTER_CH3,	// 22
	
	// setters
	SET_TARGET_CH0,		// 23
	SET_TARGET_CH1,		// 24
	SET_TARGET_CH2,		// 25
	SET_TARGET_CH3,		// 26
	SET_TARGET_ALL,		// 27
};

void printNumbers( float arr[], const uint8_t END ) {
	const uint8_t END_DELIM = END - 1;
	
	for( uint8_t i = 0; i < END; i += 1 ) {
		Serial.print( arr[i] );
		if( i < END_DELIM ) {
			Serial.print(",");
		}
	}
	Serial.println();
}

//----- 

void printPIDCoeff(const uint8_t ch) {
	// get data
	constexpr uint8_t NUM = 3;
	float data[NUM];
	controller[ch].getPIDGains(data);
	// show label
	Serial.print("PID_");
	Serial.print(ch);
	Serial.print(": ");
	// show values
	printNumbers(data, NUM);
}

void printInLimits(const uint8_t ch) {
	// get data
	constexpr uint8_t NUM = 2;
	float data[NUM];
	controller[ch].getInputLimits(data);
	// show label
	Serial.print("IN_LIMIT_");
	Serial.print(ch);
	Serial.print(": ");
	// show values
	printNumbers(data, NUM);
}

void printOutLimits(const uint8_t ch) {
	// get data
	constexpr uint8_t NUM = 2;
	float data[NUM];
	controller[ch].getOutputLimits(data);
	// show label
	Serial.print("OUT_LIMIT_");
	Serial.print(ch);
	Serial.print(": ");
	// show values
	printNumbers(data, NUM);
}

void printAlphaBetaCoeff(const uint8_t ch) {
	// get data
	constexpr uint8_t NUM = 2;
	float data[NUM];
	controller[ch].getFilterGains(data);
	// show label
	Serial.print("AB_FILTER_");
	Serial.print(ch);
	Serial.print(": ");
	// show values
	printNumbers(data, NUM);
}

void printKalmanCoeff(const uint8_t ch) {
	// get data
	constexpr uint8_t NUM = 2;
	float data[NUM];
	filter[ch].getGains(data);
	// show label
	Serial.print("K_FILTER_");
	Serial.print(ch);
	Serial.print(": ");
	// show values
	printNumbers(data, NUM);
}

//-----

uint8_t readSerialInputs( float target[] ) {
	// -- state --
	static uint8_t output_flag = FILTER; 	// keep enum definitions encapsulated 

	if( Serial.available() > 0 ) {
		// buffer
		char buffer[N_CHAR] = {0}; 

		// get data
		constexpr char END = '\n'; 		// string end character; provided by serial input | IMPORTANT
		uint8_t num_read = Serial.readBytesUntil(END, buffer, N_CHAR); 
		
		// convert to number array
		float input[INPUTS_MAX] = {-1}; // default value
		uint8_t ninput = parseNumbers(buffer, input);
		
		// apply available functions
		float function = input[0];
		float* parameter = input + 1;
		
		// getters
		if( ninput == 1 ) {
			
			// state output || can be used to capture temperature readings (avoid state)
			if( function == GET_FILTER ) {
				output_flag = FILTER;
			}
			else if( function == GET_RAW ) {
				output_flag = RAW;
			}
			else if( function == GET_TARGET ) {
				output_flag = TARGET;
			}
			
			// PID controller
				// gains
			else if( function == GET_PID_CH0 ) {
				printPIDCoeff(0);
			}
			else if( function == GET_PID_CH1 ) {
				printPIDCoeff(1);
			}
			else if( function == GET_PID_CH2 ) {
				printPIDCoeff(2);
			}
			else if( function == GET_PID_CH3 ) {
				printPIDCoeff(3);
			}
			
				// input limits
			else if( function == GET_IN_LIMIT_CH0 ) {
				printInLimits(0); 
			}
			else if( function == GET_IN_LIMIT_CH1 ) {
				printInLimits(1); 
			}
			else if( function == GET_IN_LIMIT_CH2 ) {
				printInLimits(2); 
			}
			else if( function == GET_IN_LIMIT_CH3 ) {
				printInLimits(3); 
			}
			
				// output limits
			else if( function == GET_OUT_LIMIT_CH0 ) {
				printOutLimits(0); 
			}
			else if( function == GET_OUT_LIMIT_CH1 ) {
				printOutLimits(1); 
			}
			else if( function == GET_OUT_LIMIT_CH2 ) {
				printOutLimits(2); 
			}
			else if( function == GET_OUT_LIMIT_CH3 ) {
				printOutLimits(3); 
			}
			
			// Filters
				// Alpha-beta filters
			else if( function == GET_AB_FILTER_CH0 ) {
				printAlphaBetaCoeff(0);
			}
			else if( function == GET_AB_FILTER_CH1 ) {
				printAlphaBetaCoeff(1);
			}
			else if( function == GET_AB_FILTER_CH2 ) {
				printAlphaBetaCoeff(2);
			}
			else if( function == GET_AB_FILTER_CH3 ) {
				printAlphaBetaCoeff(3);
			}
			
				// Kalman Filters
			else if( function == GET_K_FILTER_CH0 ) {
				printKalmanCoeff(0);
			}
			else if( function == GET_K_FILTER_CH1 ) {
				printKalmanCoeff(1);
			}
			else if( function == GET_K_FILTER_CH2 ) {
				printKalmanCoeff(2);
			}
			else if( function == GET_K_FILTER_CH3 ) {
				printKalmanCoeff(3);
			}
		}
		
		// setters
		else if( ninput == 2 ) {
		
			// PID target values
			if( function == SET_TARGET_CH0 ) {
				target[0] = parameter[0];
			}
			else if( function == SET_TARGET_CH1 ) {
				target[1] = parameter[0];
			}
			else if( function == SET_TARGET_CH2 ) {
				target[2] = parameter[0];
			}
			else if( function == SET_TARGET_CH3 ) {
				target[3] = parameter[0];
			}
		}
		
		else if( ninput == 5 && function == SET_TARGET_ALL ) {
			target[0] = parameter[0];
			target[1] = parameter[1];
			target[2] = parameter[2];
			target[3] = parameter[3];
		} 
		
		
	}
	
	return output_flag;
}



//----------------- SERIAL PARAMETER CHANGES
/*
void testFunc() {
	controller[0].setPIDGains(0,0,0); // can cross-reference global objects
}*/


/*
void printParameters( void(*func_get)(float []), const uint8_t ch, const uint8_t num, const char label[] ) {
	float data[num];
	func_get(data);
	
	Serial.print("label");
	Serial.print(ch);
	Serial.print(": ");
	
	printNumbers(data, num);
}*/

//#define GET_PID_GAINS(ch) []( float x[] ) -> void { controller[ch].getPIDGains(x); }
// [] (int x) -> bool { return (x == 0); }
// auto func_get = []( float x[] ) { controller[ch].getPIDGains(x); };

/*
void printPIDGains(const uint8_t CH) {
	auto func_get = []( float x[] ) { controller[0].getPIDGains(x); };
	printParameters(func_get, 3, CH);
}

void printInLimits(const uint8_t CH) {
	auto func_get = [CH]( float x[] ) { controller[CH].getInputLimits(x); };
	printParameters(func_get, 2, CH);
}*/

//-----
