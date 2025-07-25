#include "src/BasicCN0391/BasicCN0391.h"
#include "src/PIDController/PIDController.h"
#include "src/DiscretePulseFrequency/DiscretePulseFrequency.h"
#include "src/KalmanFilter1D/KalmanFilter1D.h"
#include "src/TimerOne/TimerOne.h"  // External library: https://github.com/PaulStoffregen/TimerOne
#include "Constants.h"

/* NOTE: 
	- Open "README.md" to understand how to use code.
	- See "Constants.h" to configure program.
*/
void setup() {
	// enable serial communication
	Serial.begin(BAUD_RATE); 
	Serial.flush();
	delay(1000);
	
	Serial.println( F("CONNECTED") );
	
	// sensors
	char stype[] = {SENSOR_TYPE}; // Default values | Will wait until specified. 
	readSensorTypes(stype);
	
	// configure sensor and board
		// controllers
	float temp_av[NUM_PORT];
	setupControllers(temp_av, stype);
	setupFilters(temp_av); 
	
		// interrupt
	Timer1.initialize(PULSE_WIDTH);
	Timer1.attachInterrupt(updateSignals);
	
	Serial.println( F("CALIBRATED") );
	Serial.flush(); 
}

void loop() {
	// read sensors
	float measure[NUM_PORT];
	CN391_getThermocoupleTemps(measure);
	
	// get target state
	static float target[NUM_PORT] = {PID_TARGET};                      // target temperature
	static bool enable_pid[NUM_PORT] = {false, false, false, false};   // default to off
	
	readSerialInputs(target, measure, enable_pid);
	
	// adjust output
	updateControllers(target, measure, enable_pid); 
	
	// return measurements
	updateOutputFilters(measure);
}

