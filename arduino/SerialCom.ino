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

constexpr int INPUTS_MAX = 6; // <command> <channel> <param1> <param2> <param3> <param4> 


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

enum CHANNEL {
	CH0,	// 0
	CH1,	// 1
	CH2,	// 2
	CH3,	// 3
	CH_ALL	// 4
};

enum FUNCTION {
	// getters
		// output
	GET_FILTER,			// 0
	GET_RAW,			// 1
	GET_TARGET,			// 2
		// pid
	GET_PID,			// 3
	GET_IN_LIMIT,		// 4
	GET_OUT_LIMIT,		// 5
		// filters
	GET_AB_FILTER,		// 6
	GET_K_FILTER,		// 7

	// setters
		// targets
	SET_TARGET,			// 8
		// pid
	SET_PID,			// 9
	SET_IN_LIMIT,		// 10
	SET_OUT_LIMIT,		// 11
		// filter
	SET_AB_FILTER,		// 12
	SET_K_FILTER,		// 13
	SET_K_FILTER_STATE	// 14
};

//---

void printNumbers( float arr[], const uint8_t END ) {
	const uint8_t END_DELIM = END - 1;
	// elements
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
		int function = input[0];		// truncate
		int channel = input[1];			// truncate
		float* parameter = input + 2;
		
		// getters
		if( ninput == 1 ) {
			
			// state output | can be used to capture temperature readings (avoid state)
			if( function == GET_FILTER ) {
				output_flag = FILTER;
			}
			else if( function == GET_RAW ) {
				output_flag = RAW;
			}
			else if( function == GET_TARGET ) {
				output_flag = TARGET;
			}
		}
		
			// check valid channel
		if( channel == CH0 || channel == CH1 || channel == CH2 || channel == CH3 ) {

			if( ninput == 2 ) {
				// PID controller
					// gains
				if( function == GET_PID ) {
					printPIDCoeff(channel);
				}
				
					// input limits
				else if( function == GET_IN_LIMIT ) {
					printInLimits(channel); 
				}
				
					// output limits
				else if( function == GET_OUT_LIMIT ) {
					printOutLimits(channel); 
				}
				
				// Filters
					// Alpha-beta filters
				else if( function == GET_AB_FILTER ) {
					printAlphaBetaCoeff(channel);
				}
				
					// Kalman Filters
				else if( function == GET_K_FILTER ) {
					printKalmanCoeff(channel);
				}
			}
			
			// setters
			else if( ninput == 3 ) {
			
				// target values (per channel)
				if( function == SET_TARGET ) {
					target[channel] = parameter[0];
				}
				
				// alpha-beta filter (1 input):
				else if( function == SET_AB_FILTER ) {
					controller[channel].setFilterGains( parameter[0] );
				}
				
				// kalman filter (set state)
				else if( function == SET_K_FILTER_STATE ) {
					filter[channel].setState( parameter[0] );
				}
			}
	 		
			else if( ninput == 4 ) {
				// pid inputs
				if( function == SET_IN_LIMIT ) {
					controller[channel].setInputLimits( parameter[0], parameter[1] );
				}
				
				// pid outputs
				else if( function == SET_OUT_LIMIT ) {
					controller[channel].setOutputLimits( parameter[0], parameter[1] );
				}
				
				// filters (2 parameters)
					// alpha-beta 
				else if( function == SET_AB_FILTER ) {
					controller[channel].setFilterGains( parameter[0], parameter[1] );
				}
				
					// kalman filter [gains]
				else if( function == SET_K_FILTER ) {
					filter[channel].setGains( parameter[0], parameter[1] );
				}
			}
			
			// PID coefficients
			else if( ninput == 5 && function == SET_PID ) {
				controller[channel].setPIDGains( parameter[0], parameter[1], parameter[2] );
			}
		}
		
		// target value for all channels
		if( ninput == 6 && function == SET_TARGET && channel == CH_ALL ) {
			target[0] = parameter[0];
			target[1] = parameter[1];
			target[2] = parameter[2];
			target[3] = parameter[3];
		}
	}
	
	return output_flag;
}

