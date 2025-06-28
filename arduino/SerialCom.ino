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
	const char delim[] = ",";			// delimiter is string | IMPORTANT
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

	// buffer
	char buffer[N_CHAR]; 

	if( Serial.available() > 0 ) {
		// clear buffer [necessary if string changes length]
		constexpr int N_BYTE = sizeof(buffer);
		memset(buffer, 0, N_BYTE);

		// get data
		const char end = '\n'; 		// string end character; provided by serial input | IMPORTANT
		int num_read = Serial.readBytesUntil(end, buffer, N_CHAR); 

		// capture output type
		if( strcmp(buffer, "raw") == 0 ) {
			output_flag = RAW;
		
		} else if( strcmp(buffer, "filter") == 0 ) {
			output_flag = FILTER;
		
		} else if( strcmp(buffer, "target") == 0 ) {
			output_flag = TARGET;
		
		} else { 	// retrieve target states
			parseNumbers(buffer, num_arr);
		} 
	}
	
	return output_flag;
}
