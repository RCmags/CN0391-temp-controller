#include "Commands.h"

//--------------- STRING SERIAL PRINTING ---------------

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

template <typename number_t>
void printPorts( const __FlashStringHelper * label, number_t data[] ) { // <F(label)>, <array> 
	Serial.print( label );
	Serial.print( F(DELIM_CHAR) );
	printArray(data, NUM_PORT);
}

void printParameters( const __FlashStringHelper * label, float         data[],
                      const uint8_t                  CH, const uint8_t NUM   ) {
	// show label
	Serial.print( label );
	Serial.print( CH );
	Serial.print( F(DELIM_CHAR) );
	// show values
	printArray(data, NUM);
}

void printError() {
	Serial.println( F("BAD_COMMAND") ); 
}

//--------------- STRING PARSING ---------------

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

uint8_t captureCharacters(char buffer[], const uint8_t N_CHAR) {
	return Serial.readBytesUntil(END_LINE_CHAR, buffer, N_CHAR); 
}

void parseStringCommand( char buffer[],     float target[],  float measure[], 
                         bool enable_pid[], float timeout[], float timer[]  ) {
	
	// convert to number array
	float input[INPUTS_MAX] = {-1}; // default negative value ensures no command is executed
	uint8_t ninput = parseNumbers(buffer, input);
	
	if(ninput == 0) {               // exit function if no useful data
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
			float data[] = { filter[0].value(), filter[1].value() , 
			                 filter[2].value(), filter[3].value() };
			printPorts( F("FILTER"), data);
			return;
		}
		
		else if( function == GET_RAW ) { // calls may be factored
			printPorts( F("RAW"), measure);
			return;
		}
		
		else if( function == GET_TARGET ) { 
			printPorts( F("TARGET"), target);
			return;
		}
		
		else if( function == GET_SENSOR_TYPE ) {
			char data[NUM_PORT]; CN391_getPortType(data);
			printPorts( F("SENSOR_TYPES"), data);
			return;
		}
		
		// controller state
		else if( function == GET_ENABLE ) {
			printPorts( F("ENABLE"), enable_pid);
			return;
		}
		
		// timer
		else if( function == GET_TIMER ) {
			printPorts( F("TIMER"), timer);
			return;
		}
		else if( function == GET_TIMEOUT ) { 
			printPorts( F("TIMEOUT"), timeout);
			return;
		}
	}
	
		// check valid channel
	else if( channel == CH0 || channel == CH1 || channel == CH2 || channel == CH3 ) {

		if( ninput == 2 ) {
			// PID controller
				// gains
			if( function == GET_PID ) { 
				float data[3]; controller[channel].getPIDGains(data);
				printParameters( F("PID_"), data, channel, 3 );
				return;
			}
				// input limits
			else if( function == GET_IN_LIMIT ) { 
				float data[2]; controller[channel].getInputLimits(data);
				printParameters( F("IN_LIMIT_"), data, channel, 2 );
				return;
			}
			
			// Filters
				// Alpha-beta filters
			else if( function == GET_AB_FILTER ) {
				float data[2]; controller[channel].getFilterGains(data);
				printParameters( F("AB_FILTER_"), data, channel, 2 );
				return;
			}
			
				// Kalman Filters
			else if( function == GET_K_FILTER ) {
				float data[2]; filter[channel].getGains(data);
				printParameters( F("K_FILTER_"), data, channel, 2 );
				return;
			}
			
			// timer
			else if( function == SET_ENABLE ) {
				enable_pid[channel] = true;
				Serial.println( F("SET_ENABLE") );
				return;
			}
			else if( function == SET_DISABLE ) {
				enable_pid[channel] = false;
				timer[channel] = 0; 			// reset time
				Serial.println( F("SET_DISABLE") );
				return;
			}
		}
		
		// setters
		else if( ninput == 3 ) {
		
			// target values (per channel)
			if( function == SET_TARGET ) {
				target[channel] = parameter[0];
				Serial.println( F("SET_TARGET") );
				return;
			}
			
			// alpha-beta filter (1 input):
			else if( function == SET_AB_FILTER ) {
				controller[channel].setFilterGains( parameter[0] );
				Serial.println( F("SET_AB_FILTER") );
				return;
			}
			
			// kalman filter (set state)
			else if( function == SET_K_FILTER_STATE ) {
				filter[channel].setState( parameter[0] );
				Serial.println( F("SET_K_FILTER_STATE") );
				return;
			}
			
			// update timeout
			else if(  function == SET_TIMEOUT && parameter[0] > 0 || parameter[0] == TIME_INF ) {
				timeout[channel] = parameter[0];
				Serial.println( F("SET_TIMEOUT") );
				return;
			}
		}
 		
		else if( ninput == 4 ) {
			// pid inputs
			if( function == SET_IN_LIMIT ) {
				controller[channel].setInputLimits( parameter[0], parameter[1] );
				Serial.println( F("SET_IN_LIMIT") );
				return;
			}
			
			// filters (2 parameters)
				// alpha-beta 
			else if( function == SET_AB_FILTER ) {
				controller[channel].setFilterGains( parameter[0], parameter[1] );
				Serial.println( F("SET_AB_FILTER") );
				return;
			}
			
				// kalman filter [gains]
			else if( function == SET_K_FILTER ) {
				filter[channel].setGains( parameter[0], parameter[1] );
				Serial.println( F("SET_K_FILTER") );
				return;
			}
		}
		
		// PID coefficients
		else if( ninput == 5 && function == SET_PID ) {
			controller[channel].setPIDGains( parameter[0], parameter[1], parameter[2] );
			Serial.println( F("SET_PID") );
			return;
		}
	}
	
	// change all ports
	else if( channel == CH_ALL ) {
	
		if( ninput == 6 && function == SET_TARGET ) {
			// target value for all channels
			for( uint8_t ch = 0; ch < NUM_PORT; ch += 1 ) {
				target[ch] = parameter[ch];
			}
			Serial.println( F("SET_TARGET") );
			return;
		}
		
		else if( ninput == 2 ) {
			// timer 
				// enable
			if( function == SET_ENABLE ) {
				for( uint8_t ch = 0; ch < NUM_PORT; ch += 1 ) {
					enable_pid[ch] = true;
				}
				Serial.println( F("SET_ENABLE") );
				return;
			}
			
				// dissable
			else if( function == SET_DISABLE ) {
				for( uint8_t ch = 0; ch < NUM_PORT; ch += 1 ) {
					enable_pid[ch] = false;
					timer[ch] = 0; 			// reset time
				}
				Serial.println( F("SET_DISABLE") );
				return;
			}
		}
	}
	
	// indicate bad response if no command was correct
	printError(); 
}

//--------------- SERIAL INPUT ---------------

void readSerialInputs( float target[], float measure[], bool enable_pid[] ) { 
	// dissable controller after timeout
	static float timer[NUM_PORT] = {0};					// total time controller has been enabled
														// time controller is allowed to be on
	static float timeout[NUM_PORT] = {TIME_INF, TIME_INF, TIME_INF, TIME_INF};

	for( uint8_t ch = 0; ch < NUM_PORT; ch += 1 ) {
		if( enable_pid[ch] ) {
			timer[ch] += PIDcontroller::getTimeStep(); 	       // measure on-time
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
		Serial.flush(); // clear buffers
	}
}

void readSensorTypes( char stype[] ) {
	Serial.println( F("WAITING-TYPES") );
	
	// Wait until data is received
	bool loop = true;	// enter loop by default
	while( loop ) {
	
		if( Serial.available() > 0 ) {
			// capture character array: 	[TYPE, TYPE, TYPE, TYPE]
			char buffer[BUFFER_SIZE] = {0};
			uint8_t num_read = captureCharacters(buffer, BUFFER_SIZE);

			if( num_read == NUM_PORT ) {	// one character per port
				// check characters
				uint8_t ch;
				for( ch = 0; ch < NUM_PORT; ch += 1 ) {
					char type = buffer[ch];
					bool isValid = CN391_checkPortType( type );
					
					// indicate result
					if( !isValid ) {
						printError(); 
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
			
			// indicate bad response 
			else {
				printError();
			}
			
			Serial.flush(); // clear buffers
		}
	}
}

