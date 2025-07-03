//#include "src/BasicCN0391/BasicCN0391.h"
#include "Commands.h"

// NOTE: use F() macro to drastically reduce RAM usage. 
// See: https://support.arduino.cc/hc/en-us/articles/360013825179-Reduce-the-size-and-memory-usage-of-your-sketch

//--------------- 

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


//--------------- 

template <typename number_t>
void printArray( number_t arr[], const uint8_t END ) {
	const uint8_t END_DELIM = END - 1;
	// elements
	for( uint8_t i = 0; i < END; i += 1 ) {
		Serial.print( arr[i] );
		if( i < END_DELIM ) {
			Serial.print( F(", ") ); // use space?
		}
	}
	Serial.println();
}

//--------------- 

//--

void printPIDCoeff(const uint8_t ch) {
	// get data
	constexpr uint8_t NUM = 3;
	float data[NUM];
	controller[ch].getPIDGains(data);
	// show label
	Serial.print( F("PID_") );
	Serial.print(ch);
	Serial.print( F(": ") );
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
	Serial.print( F(": ") );
	// show values
	printArray(data, NUM);
}

void printOutLimits(const uint8_t ch) {
	// get data
	constexpr uint8_t NUM = 2;
	float data[NUM];
	controller[ch].getOutputLimits(data);
	// show label
	Serial.print( F("OUT_LIMIT_") );
	Serial.print(ch);
	Serial.print( F(": ") );
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
	Serial.print( F(": ") );
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
	Serial.print( F(": ") );
	// show values
	printArray(data, NUM);
}

//--------------- 

// -- state --
//static uint8_t output_flag = FILTER; 	// keep enum definitions encapsulated 
// must make global state. make read commands independent of loop

void readSerialInputs( float target[], float measure[] ) { 
	
	if( Serial.available() > 0 ) { // change to while to ensure multiple readings -> add timeout
		// get data
		constexpr char END = '\n'; 		// string end character; provided by serial input | IMPORTANT
		
		char buffer[N_CHAR] = {0};
		uint8_t num_read = Serial.readBytesUntil(END, buffer, N_CHAR); 
		
		// convert to number array
		float input[INPUTS_MAX] = {-1}; // default value
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
				Serial.print( F("FILTER: ") );
				float data[] = { filter[0].value(), 
				                 filter[1].value(), 
				                 filter[2].value(),
				                 filter[3].value() };
				printArray(data, NUM_PORT); // NUM_PORT
			}
			else if( function == GET_RAW ) {
				Serial.print( F("RAW: ") );
				printArray(measure, NUM_PORT); // NUM_PORT
			}
			else if( function == GET_TARGET ) {
				Serial.print( F("TARGET: ") );
				printArray(target, NUM_PORT); // NUM_PORT
			}
			
			else if( function == GET_SENSOR_TYPE ) {
				Serial.print( F("SENSOR_TYPES: ") );
				char data[NUM_PORT]; 
				CN391_getPortType(data);
				printArray(data, NUM_PORT); // NUM_PORT
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
		
		
		if( ninput == 6 && channel == CH_ALL ) {
			
			// target value for all channels
			if( function == SET_TARGET ) {
				target[0] = parameter[0];
				target[1] = parameter[1];
				target[2] = parameter[2];
				target[3] = parameter[3];
			}
			
			// sensor types
			else if( function == SET_SENSOR_TYPE ) {
				char data[] = { uint8_t(parameter[0]),
				                uint8_t(parameter[1]),
				                uint8_t(parameter[2]),
				                uint8_t(parameter[3]) };
				printArray(data, 4);
				/*
				char data2[] = {'J', 'J', 'J', 'J'};
				CN391_setup(data2);
				delay(500); // wait for sensor to update [blocking]
				*/
				// ^ The above blocks execution 
			}
		}
	}
}

//--------------- 

// need to check if connection is established. setup ping function. 

void readSensorTypes() {
	Serial.println( F("WAITING-TYPES") );

	bool loop = true;
	while( loop ) {
		//Serial.println("Waiting for command");
		//delay(1000);
		
		if( Serial.available() > 0 ) {
			//Serial.println("Received types");
		
			constexpr char END = '\n';
			char buffer[N_ENABLED] = {0};
			uint8_t num_read = Serial.readBytesUntil(END, buffer, N_CHAR); 
			
			//Serial.println(num_read);
			
			if( num_read == 4 ) {
				
				uint8_t ch;
				for( ch = 0; ch < N_ENABLED; ch += 1 ) {
					char type = buffer[ch];
					bool isValid = CN391_checkPortType( type );
					//Serial.print(type);
					
					if( !isValid ) {
						break;
						Serial.println( F("INVALID-TYPES") );
					}
 				}
 				
 				if( ch == N_ENABLED ) {
 					loop = false;
 					Serial.println( F("RECEIVED-TYPES") );
 				}
			}
		}
	}
}

