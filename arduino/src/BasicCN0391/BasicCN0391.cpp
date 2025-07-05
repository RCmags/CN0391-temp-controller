// General dependencies
#include <Arduino.h>
#include <SPI.h>

// Sensor shield 
#include "../CN0391/CN0391.h"
#include "../CN0391/Communication.h"
#include "../CN0391/Thermocouple.h"

//============ Functions ==============
#include "BasicCN0391.h"

// Declare the global ADC reading array and thermocouple type array from the CN0391 library
extern int32_t _ADCValue1[];
extern uint8_t th_types[NUM_PORT];
uint8_t sensor_types[NUM_PORT];

// Parser to isolate constants from "thermocouple.h"
uint8_t setType(char type) {
	switch(type) {
		case 'T':
			return TYPE_T;
			break;
		case 'J':
			return TYPE_J;
			break;
		case 'K':
			return TYPE_K;
			break;
		case 'E':
			return TYPE_E;
			break;
		case 'S':
			return TYPE_S;
			break;
		case 'R':
			return TYPE_R;
			break;
		case 'N':
			return TYPE_N;
			break;
		default: 				// Type B
			return TYPE_B;
	}
}

char setTypeInv(uint8_t type) {
	switch(type) {
		case TYPE_T:
			return 'T';
			break;
		case TYPE_J:
			return 'J';
			break;
		case TYPE_K:
			return 'K';
			break;
		case TYPE_E:
			return 'E';
			break;
		case TYPE_S:
			return 'S';
			break;
		case TYPE_R:
			return 'R';
			break;
		case TYPE_N:
			return 'N';
			break;
		default: 				// Type B
			return 'B';
	}
}

// port types
bool CN391_checkPortType(char type) {
	return type == 'T' || type == 'J' || type == 'K' || type == 'E' ||
	       type == 'S' || type == 'R' || type == 'N' || type == 'B' ;
}

void CN391_getPortType( char types[] ) {
	for( uint8_t ch = 0; ch < NUM_PORT; ch += 1 ) {
		types[ch] = setTypeInv( sensor_types[ch] );
	}
} 

//--- main functions ---

void CN391_setup( char port[] ) { // vectorize to array
	// SPI setup
	SPI.begin();
	SPI.setDataMode(SPI_MODE3);
	pinMode(CS_PIN, OUTPUT);
	digitalWrite(CS_PIN, HIGH);

	// Set all thermocouple channels to given inputs
	for( uint8_t ch = 0; ch < NUM_PORT; ch += 1 ) {
		th_types[ch] = setType( port[ch] );
		sensor_types[ch] = th_types[ch];
	}

	// Calibrate the RTD (cold junction) and thermocouple channels
	CN0391_init();
	CN0391_calibration(RTD_CHANNEL);
	CN0391_calibration(TH_CHANNEL);
}

void CN391_setup( char port ) {		// Over-ride for identical types
	char arr[] = {port, port, port, port};
	CN391_setup(arr);
}

// get all temps
void CN391_getTemps(float cjTemp[], float tcTemp[], float tcVoltage[], int32_t tcRaw[] ) {
	// Update all ADC data (both RTD and thermocouple channels)
	CN0391_set_data();

	for( uint8_t i = 0; i < NUM_PORT; i++ ) {
		// Get the cold junction temperature using the RTD channel corresponding to port i
		CN0391_calc_rtd_temperature(i, &cjTemp[i]);

		// Read the thermocouple raw ADC value from the global array (_ADCValue1)
		tcRaw[i] = _ADCValue1[i];

		// Convert the raw ADC value to voltage using the TH_CHANNEL configuration
		tcVoltage[i] = CN0391_data_to_voltage(tcRaw[i], TH_CHANNEL);

		// Calculate the thermocouple temperature using the cold junction temperature
		CN0391_calc_th_temperature(i, cjTemp[i], &tcTemp[i]);
	}
}

// get probe temps
void CN391_getThermocoupleTemps(float* tcTemp) {
	// Update all ADC data 
	CN0391_set_data();

	// For each port (P1 to P4) update readings:
	for( uint8_t i = 0; i < NUM_PORT; i++ ) {
		float cjTemp;
		CN0391_calc_rtd_temperature(i, &cjTemp);
		CN0391_calc_th_temperature(i, cjTemp, &tcTemp[i]); 	// return tcTemp
	}
}
