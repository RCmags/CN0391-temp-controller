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
	Serial.begin(BAUD_RATE); // output to mention connection established / enable other code if so
	Serial.flush();
	
	Serial.println("Established connection");
	
	// configure sensor and board
	float temp_av[NUM_PORT];
	char stype[] = {SENSOR_TYPE};

	setupControllers(temp_av, stype);
	setupFilters(temp_av);
	
	Serial.println("Calibrated");
}

void loop() {
	// get target state
	static float target[N_ENABLED] = {0}; 			// target temperature | initialized at zero
	uint8_t output_flag = readSerialInputs(target); // state output; default to filter

	// adjust output
	float measure[NUM_PORT];
	updateControllers(target, measure);

	// return measurements
	updateOutputFilters(measure);
	sendSerialOutput(measure, target, output_flag);
}

