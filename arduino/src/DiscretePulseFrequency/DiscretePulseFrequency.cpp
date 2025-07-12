#include "Arduino.h"
#include "DiscretePulseFrequency.h"

void DiscretePulseFrequency::setup() {
	pinMode(PIN, OUTPUT);
	digitalWrite(PIN, LOW);
	cycle_count = 0;
}

void DiscretePulseFrequency::update(float duty) {
	// count polling calls
	cycle_count += 1;

	// saturate output if input saturates
	bool pulse;
	if( duty <= 0 ) {
		cycle_count = 0;
		pulse = LOW;

	} else if ( duty >= 1 ) {
		cycle_count = 0;
		pulse = HIGH;

	// Generate pulses if output not saturated
	} else {
		// ideal continous timing
		float gain;
		if( duty < 0.5 ) { 
			gain = (1 - duty)/duty;
		} else {
			gain = duty/(1 - duty);
		}

		// truncate timing to interger steps (assumes constant polling time)
		int count = floor(gain);

		if( cycle_count <= count ) { 
			pulse = LOW;
		} else {
			pulse = HIGH;
			cycle_count = 0;
		}

		// Shift transition above 0.5 to avoid random flips
		constexpr float DUTY_FLIP = 2.0/3.0;
		if( duty >= DUTY_FLIP ) {
			pulse = !pulse; 	// reverse bit logic
		}
	}

	// bit-banging to generate pulse frequency signal
	digitalWrite(PIN, pulse);
}

// use external setter to define output
void DiscretePulseFrequency::update() {
	update(_duty);
}

void DiscretePulseFrequency::setDuty(float x) {
	_duty = x;		// modify variable
}


