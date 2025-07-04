//----------------- ARRAYS OF CLASSES

PIDcontroller controller[] = {
	PIDcontroller(PID_COEFF_1), 
	PIDcontroller(PID_COEFF_2), 
	PIDcontroller(PID_COEFF_3), 
	PIDcontroller(PID_COEFF_4), 
};

DiscretePulseFrequency signal[] = {
	DiscretePulseFrequency(PID_PIN_1),
	DiscretePulseFrequency(PID_PIN_2),
	DiscretePulseFrequency(PID_PIN_3),
	DiscretePulseFrequency(PID_PIN_4),
};

KalmanFilter1D filter[] = {
	KalmanFilter1D(FILT_COEFF_1),
	KalmanFilter1D(FILT_COEFF_2),
	KalmanFilter1D(FILT_COEFF_3),
	KalmanFilter1D(FILT_COEFF_4),
};

//----------------- PID CONTROLLER FUNCTIONS

void setupControllers( float temp_av[], char stype[] ) {
	// set state
	CN391_setup(stype);

	PIDcontroller::initTimer();
	delay(1000); 						// add enough delay to ensure board is on

	// Calculate calibration average
	for( uint8_t ch = 0; ch < NUM_PORT; ch += 1 ) {
		temp_av[ch] = 0;				// must initialize to zero
	}
	
	for( uint8_t i = 0; i < N_AVERAGE; i += 1) {
		float tcTemp[NUM_PORT]; 
		CN391_getThermocoupleTemps(tcTemp);
	
		for( uint8_t ch = 0; ch < NUM_PORT; ch += 1 ) {
			temp_av[ch] += tcTemp[ch] / float(N_AVERAGE);
		}
	}

	// set filter states	
	for( uint8_t ch = 0; ch < NUM_PORT; ch += 1 ) {
		controller[ch].setState( temp_av[ch] );		// initalized to normalized average
		signal[ch].setup();
	}
}

void setupFilters( float temp_av[] ) {
	for( uint8_t ch = 0; ch < NUM_PORT; ch += 1 ) {
		filter[ch].setState( temp_av[ch] ); 		// initialize filters to current state
	}
}

void updateControllers( float target[], float measure[], bool enable ) {
	// update state
	PIDcontroller::updateTimeStep();

	// controller
	for( uint8_t ch = 0; ch < NUM_PORT; ch += 1 ) {
		if( enable ) {
			controller[ch].update( target[ch], measure[ch] );
			signal[ch].update( controller[ch].output() ); 		// send Pulse Frequency modulated signal
		} else {
			controller[ch].setState( measure[ch] );				// reset controller
			signal[ch].update(0); 								// disable pins
		}
	}
}

void updateOutputFilters( float measure[] ) {
	// filter raw values for serial data
	for( uint8_t ch = 0; ch < NUM_PORT; ch += 1 ) {
		filter[ch].update( measure[ch] );
	}
}

