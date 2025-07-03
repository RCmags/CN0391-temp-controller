#include "src/BasicCN0391/BasicCN0391.h"
#include "src/PIDController/PIDController.h"
#include "src/DiscretePulseFrequency/DiscretePulseFrequency.h"
#include "src/KalmanFilter1D/KalmanFilter1D.h"
#include "Constants.h"

// 10k pull down resistor

/* NOTE: 
	- Open "README.md" to understand how to use code.
	- See "Constants.h" to configure program.
*/
void setup() {
	// enable serial communication
	Serial.begin(BAUD_RATE); 
	delay(1000);
	Serial.flush();
	Serial.println( F("CONNECTED") );
	
	// force pins low [dissable offset]
	digitalWrite(PID_PIN_1, LOW);
	digitalWrite(PID_PIN_2, LOW);
	digitalWrite(PID_PIN_3, LOW);
	digitalWrite(PID_PIN_4, LOW);
	
	// sensors
	readSensorTypes();
	
	// configure sensor and board
	float temp_av[NUM_PORT];
	char stype[] = {SENSOR_TYPE}; // -> need to change during start. Add character read loop. Wait for until specified. 
	
	setupControllers(temp_av, stype);
	setupFilters(temp_av); 
	Serial.println( F("CALIBRATED") );
}

void loop() {
	// read sensors
	float measure[NUM_PORT];
	CN391_getThermocoupleTemps(measure);

	// get target state
	static float target[N_ENABLED] = {PID_TARGET}; 	// target temperature
	readSerialInputs(target, measure);

	// adjust output
	updateControllers(target, measure);

	// return measurements
	updateOutputFilters(measure);
}

