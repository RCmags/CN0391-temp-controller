//----------------- SERIAL COMMUNICATION FUNCTIONS 

// Commands used to retrieve data from serial
enum SERIAL_TYPE {
	RAW,
	FILTER,
	TARGET
};

// add communication timeout
void sendSerialOutput( float measure[], float target[], uint8_t output_flag ) {
	for( int ch = 0; ch < N_ENABLED; ch += 1 ) {
		
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

	// parse string of numbers: num1, num2, num3 ...
void parseNumbers( char buffer[], float num_arr[] ) { 
	// Split string
	char* pointer;
	const char delim[] = ",";			// INPUT delimiter is string | IMPORTANT
	pointer = strtok(buffer, delim); 
	
	int index = 0;
	while( (pointer != nullptr) && (index < N_ENABLED) ) {
		// attempt conversion to double/float
		char * end_ptr;
		float number = strtod(pointer, &end_ptr); 
		
		if (*end_ptr == '\0') { 		// store if valid conversion
			num_arr[index] = number;
		}
		index += 1; // update index based on commas (fixed array position)
		
		// last call
		pointer = strtok(nullptr, delim);
	}
}

uint8_t readSerialInputs( float num_arr[] ) {
	// -- state --
	static uint8_t output_flag = FILTER; 	// keep enum definitions encapsulated 

	if( Serial.available() > 0 ) {
		// buffer
		char buffer[N_CHAR] = {0}; 

		// get data
		const char END = '\n'; 		// string end character; provided by serial input | IMPORTANT
		int num_read = Serial.readBytesUntil(END, buffer, N_CHAR); 

		// capture output type
		if( !strcmp(buffer, "raw") ) {
			output_flag = RAW;
		
		} else if( !strcmp(buffer, "filter") ) {
			output_flag = FILTER;
		
		} else if( !strcmp(buffer, "target") ) {
			output_flag = TARGET;
		
		} else if( !strcmp(buffer, "target") ) {
		
		} else { 	// retrieve target states
			parseNumbers(buffer, num_arr);
		} 
	}
	
	return output_flag;
}



//----------------- SERIAL PARAMETER CHANGES

void testFunc() {
	controller[0].setPIDGains(0,0,0); // can cross-reference global objects
}

