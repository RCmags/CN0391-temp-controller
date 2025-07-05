#include "Commands.h"

//--------------- 

	// parse string of numbers: num1, num2, num3 ...
uint8_t parseNumbers( char buffer[], float num_arr[] ) { 
	// Split string
	char* pointer;
	pointer = strtok(buffer, DELIM_CHAR); 
	
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
		pointer = strtok(nullptr, DELIM_CHAR);
	}
	return index;
}

//--------------- 

template <typename number_t>
void printArray( number_t arr[], const uint8_t END ) {
	const uint8_t END_DELIM = END - 1;
	// elements
	for( uint8_t i = 0; i < END; i += 1 ) {
		Serial.print( arr[i] );
		if( i < END_DELIM ) {
			Serial.print( F(DELIM_CHAR) ); 
		}
	}
	Serial.println();
}

//--------------- 

void printPIDCoeff(const uint8_t ch) {
	// get data
	constexpr uint8_t NUM = 3;
	float data[NUM];
	controller[ch].getPIDGains(data);
	// show label
	Serial.print( F("PID_") );
	Serial.print(ch);
	Serial.print( F(DELIM_CHAR) );
	// show values
	printArray(data, NUM);
}

void printInLimits(const uint8_t ch) {
	// get data
	constexpr uint8_t NUM = 2;
	float data[NUM];
	controller[ch].getInputLimits(data);
	// show label
	Serial.print( F("IN_LIMIT_") );
	Serial.print(ch);
	Serial.print( F(DELIM_CHAR) );
	// show values
	printArray(data, NUM);
}

void printAlphaBetaCoeff(const uint8_t ch) {
	// get data
	constexpr uint8_t NUM = 2;
	float data[NUM];
	controller[ch].getFilterGains(data);
	// show label
	Serial.print( F("AB_FILTER_") );
	Serial.print(ch);
	Serial.print( F(DELIM_CHAR) );
	// show values
	printArray(data, NUM);
}

void printKalmanCoeff(const uint8_t ch) {
	// get data
	constexpr uint8_t NUM = 2;
	float data[NUM];
	filter[ch].getGains(data);
	// show label
	Serial.print( F("K_FILTER_") );
	Serial.print(ch);
	Serial.print( F(DELIM_CHAR) );
	// show values
	printArray(data, NUM);
}

//--------------- 

// NOTE: end character is provided by serial input | IMPORTANT

uint8_t captureCharacters(char buffer[], const uint8_t N_CHAR) {
	constexpr char END = '\n'; 			// end character
	return Serial.readBytesUntil(END, buffer, N_CHAR); 
}

//--------------- 

void parseStringCommand( char buffer[],     float target[],  float measure[], 
                         bool enable_pid[], float timeout[], float timer[]  ) 

{
	// convert to number array
	float input[INPUTS_MAX] = {-1}; // default value | outside of enum range
	uint8_t ninput = parseNumbers(buffer, input);
	
	if(ninput == 0) { // exit function if no useful data
		return;	
	}
	
	// apply available functions
	int function = input[0];		// truncate
	int channel = input[1];			// truncate
	float* parameter = input + 2;
	
	// getters
	if( ninput == 1 ) {
		
		// state output | can be used to capture temperature readings (avoid state)
		if( function == GET_FILTER ) {
			Serial.print( F("FILTER") );
			Serial.print( F(DELIM_CHAR) );
			
			float data[] = { filter[0].value(), 
			                 filter[1].value(), 
			                 filter[2].value(),
			                 filter[3].value() };
			
			printArray(data, NUM_PORT); 
		}
		
		else if( function == GET_RAW ) { // calls may be factored
			Serial.print( F("RAW") );
			Serial.print( F(DELIM_CHAR) );
			printArray(measure, NUM_PORT);
		}
		
		else if( function == GET_TARGET ) {
			Serial.print( F("TARGET") );
			Serial.print( F(DELIM_CHAR) );
			printArray(target, NUM_PORT); 
		}
		
		else if( function == GET_SENSOR_TYPE ) {
			Serial.print( F("SENSOR_TYPES") );
			Serial.print( F(DELIM_CHAR) );
			char data[NUM_PORT]; 
			CN391_getPortType(data);
			printArray(data, NUM_PORT); 
		}
		
		// controller state
		else if( function == GET_ENABLE ) {
			Serial.print( F("ENABLE") );
			Serial.print( F(DELIM_CHAR) );
			printArray(enable_pid, NUM_PORT); 
		}
		
		// timer
		else if( function == GET_TIMER ) {
			Serial.print( F("TIMER") );
			Serial.print( F(DELIM_CHAR) );
			printArray(timer, NUM_PORT); 
		}
		else if( function == GET_TIMEOUT ) {
			Serial.print( F("TIMEOUT") );
			Serial.print( F(DELIM_CHAR) );
			printArray(timeout, NUM_PORT); 
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
			
			// Filters
				// Alpha-beta filters
			else if( function == GET_AB_FILTER ) {
				printAlphaBetaCoeff(channel);
			}
			
				// Kalman Filters
			else if( function == GET_K_FILTER ) {
				printKalmanCoeff(channel);
			}
			
			// timer
			else if( function == SET_ENABLE ) {
				enable_pid[channel] = true;
			}
			else if( function == SET_DISABLE ) {
				enable_pid[channel] = false;
				timer[channel] = 0; // reset time
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
			
			// update timeout
			else if(  function == SET_TIMEOUT && parameter[0] > 0 || parameter[0] == TIME_INF ) {
				timeout[channel] = parameter[0];
			}
		}
 		
		else if( ninput == 4 ) {
			// pid inputs
			if( function == SET_IN_LIMIT ) {
				controller[channel].setInputLimits( parameter[0], parameter[1] );
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
	
	// change all ports
	if( channel == CH_ALL ) {
	
		if( ninput == 6 && function == SET_TARGET ) {
			// target value for all channels
			target[0] = parameter[0];
			target[1] = parameter[1];
			target[2] = parameter[2];
			target[3] = parameter[3];
		}
		
		// timer
		else if( ninput == 2 && function == SET_ENABLE ) {
			for( uint8_t ch = 0; ch += 1; ch < NUM_PORT ) {
				enable_pid[ch] = true;
			}
		}
		else if( ninput == 2 && function == SET_DISABLE ) {
			for( uint8_t ch = 0; ch += 1; ch < NUM_PORT ) {
				enable_pid[ch] = false;
				timer[ch] = 0; 	// reset time
			}
		}
	}
}

//--------------- 

void readSerialInputs( float target[], float measure[], bool enable_pid[] ) { 
	// dissable controller after timeout
	static float timer[NUM_PORT] = {0};					// total time controller has been enabled
														// time controller is allowed to be on
	static float timeout[NUM_PORT] = {TIME_INF, TIME_INF, TIME_INF, TIME_INF};

	for( uint8_t ch = 0; ch < NUM_PORT; ch += 1 ) {
		if( enable_pid[ch] ) {
			timer[ch] += PIDcontroller::getTimeStep(); 	// measure on-time
		}
		if( timeout[ch] > 0 && timer[ch] > timeout[ch] ) {     // reset timers
			enable_pid[ch] = false;
			timer[ch] = 0;
		}
	}
	
	// retrieve command
	if( Serial.available() > 0 ) { 
		// get data
		char buffer[BUFFER_SIZE] = {0};
		uint8_t num_read = captureCharacters(buffer, BUFFER_SIZE);
		parseStringCommand( buffer, target, measure, enable_pid, timeout, timer );
	}
}

//--------------- 

void readSensorTypes( char stype[] ) {
	Serial.println( F("WAITING-TYPES") );
	
	// Wait until data is received
	bool loop = true;	// enter loop by default
	while( loop ) {
	
		if( Serial.available() > 0 ) {
		
			// capture character array: 	[TYPE, TYPE, TYPE, TYPE]
			constexpr uint8_t N_CHAR = NUM_PORT + 1; // include end character
			
			char buffer[N_CHAR] = {0};
			uint8_t num_read = captureCharacters(buffer, N_CHAR);
			
			if( num_read == NUM_PORT ) {	// one character per port
				// check characters
				uint8_t ch;
				for( ch = 0; ch < NUM_PORT; ch += 1 ) {
					char type = buffer[ch];
					bool isValid = CN391_checkPortType( type );
					
					// indicate result
					if( !isValid ) {
						break;		// reset loop if invalid data
					} else {
						stype[ch] = type;
					}
 				}
 				// succesful data
 				if( ch == NUM_PORT ) {
 					loop = false;	// exit loop
 					Serial.println( F("RECEIVED-TYPES") );
 				}
			}
			
			// exit loop
			else if( num_read == 1 && buffer[0] == '0' ) {	// use default value
				Serial.println( F("DEFAULT-TYPES") );
				break;
			}
		}
	}
}

