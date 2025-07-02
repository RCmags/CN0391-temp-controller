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

enum CHANNEL {
	CH0,	// 0
	CH1,	// 1
	CH2,	// 2
	CH3		// 3
};

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
		// targets
	SET_TARGET_CH0,		// 23
	SET_TARGET_CH1,		// 24
	SET_TARGET_CH2,		// 25
	SET_TARGET_CH3,		// 26
	SET_TARGET_ALL,		// 27
	
		// pid
	SET_PID_CH0,		// 28
	SET_PID_CH1,		// 29
	SET_PID_CH2,		// 30
	SET_PID_CH3,		// 31
	
	SET_IN_LIMIT_CH0,	// 32
	SET_IN_LIMIT_CH1,	// 33
	SET_IN_LIMIT_CH2,	// 34
	SET_IN_LIMIT_CH3,	// 35
	
	SET_OUT_LIMIT_CH0,	// 36
	SET_OUT_LIMIT_CH1,	// 37
	SET_OUT_LIMIT_CH2,	// 38
	SET_OUT_LIMIT_CH3,	// 39
	
		// filter
	SET_AB_FILTER_CH0,	// 40
	SET_AB_FILTER_CH1,	// 41
	SET_AB_FILTER_CH2,	// 42
	SET_AB_FILTER_CH3,	// 43
	
	SET_K_FILTER_CH0,	// 44
	SET_K_FILTER_CH1,	// 45
	SET_K_FILTER_CH2,	// 46
	SET_K_FILTER_CH3,	// 47
	
	SET_K_FILTER_STATE_CH0,	// 48
	SET_K_FILTER_STATE_CH1,	// 49
	SET_K_FILTER_STATE_CH2,	// 50
	SET_K_FILTER_STATE_CH3	// 51
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
		
			// target values (per channel)
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
			
			// alpha-beta filter (1 input):
			else if( function == SET_AB_FILTER_CH0 ) {
				controller[0].setFilterGains( parameter[0] );
			}
			else if( function == SET_AB_FILTER_CH1 ) {
				controller[1].setFilterGains( parameter[1] );
			}
			else if( function == SET_AB_FILTER_CH2 ) {
				controller[2].setFilterGains( parameter[2] );
			}
			else if( function == SET_AB_FILTER_CH3 ) {
				controller[3].setFilterGains( parameter[3] );
			}
			
			// kalman filter (set state)
			else if( function == SET_K_FILTER_STATE_CH0 ) {
				filter[0].setState( parameter[0] );
			}
			else if( function == SET_K_FILTER_STATE_CH1 ) {
				filter[1].setState( parameter[0] );
			}
			else if( function == SET_K_FILTER_STATE_CH2 ) {
				filter[2].setState( parameter[0] );
			}
			else if( function == SET_K_FILTER_STATE_CH3 ) {
				filter[3].setState( parameter[0] );
			}
		}
 
		else if( ninput == 4 ) {
		
			// PID coefficients
			if( function == SET_PID_CH0 ) {
				controller[0].setPIDGains( parameter[0], parameter[1], parameter[2] );
			}
			else if( function == SET_PID_CH1 ) {
				controller[1].setPIDGains( parameter[0], parameter[1], parameter[2] );
			}
			else if( function == SET_PID_CH2 ) {
				controller[2].setPIDGains( parameter[0], parameter[1], parameter[2] );
			}
			else if( function == SET_PID_CH3 ) {
				controller[3].setPIDGains( parameter[0], parameter[1], parameter[2] );
			}
		}
		
		else if( ninput == 3 ) {
			// pid inputs
			if( function == SET_IN_LIMIT_CH0 ) {
				controller[0].setInputLimits( parameter[0], parameter[1] );
			}
			else if( function == SET_IN_LIMIT_CH1 ) {
				controller[1].setInputLimits( parameter[0], parameter[1] );
			}
			else if( function == SET_IN_LIMIT_CH2 ) {
				controller[2].setInputLimits( parameter[0], parameter[1] );
			}
			else if( function == SET_IN_LIMIT_CH3 ) {
				controller[3].setInputLimits( parameter[0], parameter[1] );
			}
			
			// pid outputs
			else if( function == SET_OUT_LIMIT_CH0 ) {
				controller[0].setOutputLimits( parameter[0], parameter[1] );
			}
			else if( function == SET_OUT_LIMIT_CH1 ) {
				controller[1].setOutputLimits( parameter[0], parameter[1] );
			}
			else if( function == SET_OUT_LIMIT_CH2 ) {
				controller[2].setOutputLimits( parameter[0], parameter[1] );
			}
			else if( function == SET_OUT_LIMIT_CH3 ) {
				controller[3].setOutputLimits( parameter[0], parameter[1] );
			}
			
			// filters (2 parameters)
				// alpha-beta 
			else if( function == SET_AB_FILTER_CH0 ) {
				controller[0].setFilterGains( parameter[0], parameter[1] );
			}
			else if( function == SET_AB_FILTER_CH1 ) {
				controller[1].setFilterGains( parameter[0], parameter[1] );
			}
			else if( function == SET_AB_FILTER_CH2 ) {
				controller[2].setFilterGains( parameter[0], parameter[1] );
			}
			else if( function == SET_AB_FILTER_CH3 ) {
				controller[3].setFilterGains( parameter[0], parameter[1] );
			}
			
				// kalman filter [gains]
			else if( function == SET_K_FILTER_CH0 ) {
				filter[0].setGains( parameter[0], parameter[1] );
			}
			else if( function == SET_K_FILTER_CH1 ) {
				filter[1].setGains( parameter[0], parameter[1] );
			}
			else if( function == SET_K_FILTER_CH2 ) {
				filter[2].setGains( parameter[0], parameter[1] );
			}
			else if( function == SET_K_FILTER_CH3 ) {
				filter[3].setGains( parameter[0], parameter[1] );
			}
		}

		// target value for all channels
		else if( ninput == 5 && function == SET_TARGET_ALL ) {
			target[0] = parameter[0];
			target[1] = parameter[1];
			target[2] = parameter[2];
			target[3] = parameter[3];
		}
	}
	
	return output_flag;
}
