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


//----------------- SERIAL PARAMETER CHANGES


bool stringToFloat(String input, float* output) {
	// character arrays
	char* end_ptr;
	char* cstr = input.c_str();
	
	float number = strtod(cstr, &end_ptr); 
	
	// check if valid
	if (*end_ptr == '\0') { 		// valid conversion
		*output = number;
		return true;
	} else {
		return false;
	}
}

bool parseNumbers(float output[], String array[], int nstart, int nfound) {
	bool parse = true;
	
	for( int n = nstart; n < nfound; n += 1) { 
		String str = array[n];
		
		float number;
		bool isNumber = stringToFloat(str, &number);
		
		if( isNumber ) {
			int narr = n - nstart;
			output[narr] = number;
		} else {
			parse = false;	// stop parsing data if a value is wrong
			break;
		}
	}
	return parse;	// indicate if parsing succesful
}

//--------------------------

int stringParser( String input, String output[], const int NARR, const String DELIM ) {
	int index_end = 0;
	int index_start = 0;
	int index_string = 0;
	
	while( index_start >= 0 && index_string < NARR ) {
		index_start = input.indexOf(DELIM, index_end); 		// -1 when no string is found
		
		String sub_string = input.substring(index_end, index_start);
		index_end = index_start + 1;
		
		// update value
		output[index_string] = sub_string;
		index_string = index_string + 1;
	} 
	return index_string; // how many substrings were found
}

//--------------------------

void printNumbers( float arr[], int start, int end ) {
	for( int i = start; i < end; i += 1 ) {
		Serial.println( arr[i] );
	}
}

//--------------------------

void parseStringCommand( String input, uint8_t* output_flag, float target[] ) {
	
	constexpr int NARR = 7;
	String array[NARR];									// placeholder for strings
	int nfound = stringParser(input, array, NARR, " "); // split into parts
	
	Serial.print("nfound: ");
	Serial.println(nfound);
	
	if( nfound >= 2 ) {
		
		String state = array[0];
		String function = array[1];
		
		if( state == "get" ) {
			Serial.println(state);
			
			// modify filter output
			if( function == "filter" ) {
				Serial.println(function);
				*output_flag = FILTER;
			}
			else if( function == "raw" ) {
				Serial.println(function);
				*output_flag = RAW;
			}
			else if( function == "target" ) {
				Serial.println(function);
				*output_flag = TARGET;
			}
			
			// get filter gains
			else if( function == "pid:coeff" ) {
				Serial.println(function);
				//--
				for( int ch=0; ch < N_ENABLED; ch += 1) { // should cover all 4 ports
					float values[3];
					controller[ch].getPIDGains(values);
					
					Serial.print("pid:ch: ");
					Serial.println(ch);
					printNumbers(values, 0, 3);
				}
			}
			else if( function == "kfilter:coeff" ) {
				Serial.println(function);
				//--
				for( int ch=0; ch < N_ENABLED; ch += 1) { // should cover all 4 ports
					float values[2];
					filter[ch].getGains(values);
					
					Serial.print("kfilter:ch: ");
					Serial.println(ch);
					printNumbers(values, 0, 2);
				}
			}
			
			else if( function == "abfilter:coeff" ) {
				Serial.println(function);
				//--
				for( int ch=0; ch < N_ENABLED; ch += 1) { // should cover all 4 ports
					float values[2];
					controller[ch].getFilterGains(values);
					
					Serial.print("abfilter:ch: ");
					Serial.println(ch);
					printNumbers(values, 0, 2);
				}
			}
		}
		
		//----
		
		else if( state == "set" && nfound >= 3 ) {
			Serial.println(state);
			
			constexpr int NVAR = 4;
			float numbers[NVAR]; 			// maximum stored values
			
			bool parseIsGood = parseNumbers(numbers, array, 3, nfound);
			
			String str_channel = array[2];
			
			int channel = str_channel == "ch0" ?  0 :
			              str_channel == "ch1" ?  1 :
			              str_channel == "ch2" ?  2 :
			              str_channel == "ch3" ?  3 : 
			                                     -1 ; // default
			
			int nvalue = nfound - 3;
			bool isTarget = function == "target";
			
			if( str_channel == "all" && parseIsGood ) {
				Serial.println(str_channel);
				
				if( isTarget && nvalue == 4 ) { // change all target temps
					Serial.println(function);
					printNumbers(numbers, 0, nvalue);
					//--
					target[0] = numbers[0];
					target[1] = numbers[1];
					target[2] = numbers[2];
					target[3] = numbers[3];
				} 
			}
			
			else if( channel >= 0 && channel <= 3 && nfound >= 4 && parseIsGood ) { // valid channels
				
				Serial.print(str_channel);
				Serial.print(": ");
				Serial.println(channel);

				if( isTarget && nvalue == 1 ) {	// change specific channel target
					Serial.println(function);
					printNumbers(numbers, 0, nvalue);
					//--
					target[channel] = numbers[0];
				}
				
				else if( function == "pid" && nvalue == 3 ) {
					Serial.println(function);
					printNumbers(numbers, 0, nvalue);
					//--
					controller[channel].setPIDGains( numbers[0], numbers[1], numbers[2] );
				}
				
				else if( function == "kfilter" && nvalue == 2 ) {
					Serial.println(function);
					printNumbers(numbers, 0, nvalue);
					//--
					filter[channel].setGains( numbers[0], numbers[1] );
				}
				
				else if( function == "abfilter" && nvalue == 2 ) {
					Serial.println(function);
					printNumbers(numbers, 0, nvalue);
					//--
					controller[channel].setFilterGains( numbers[0], numbers[1] );
				}
			}
		}
	}
}


uint8_t readSerialInputs( float target[] ) {
	static uint8_t output_flag = FILTER;

	if( Serial.available() > 0 ) {
		String input = Serial.readString(); // cannot use line ending
		parseStringCommand(input, &output_flag, target);
		
		Serial.print("flag: ");
		Serial.println(output_flag);
	}
	return output_flag;
}
