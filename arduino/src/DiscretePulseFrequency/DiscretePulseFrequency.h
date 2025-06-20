#include "Arduino.h"

#ifndef DiscretePulseFrequency_h
#define DiscretePulseFrequency_h

class DiscretePulseFrequency {
	private:
    	const uint8_t PIN;
			uint16_t cycle_count = 0;

	public:
		// define constructor / constants:
    	DiscretePulseFrequency(uint8_t pin) : PIN(pin) {}

		// setter and getter functions:
		void setup();
		void update(float);
};

#endif
