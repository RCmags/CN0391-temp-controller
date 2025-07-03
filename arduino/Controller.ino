//----------------- ARRAYS OF CLASSES

// use preprocessor macros to program controllers in seperate file.

PIDcontroller controller[] = {
	#ifdef PID_COEFF_1
		PIDcontroller(PID_COEFF_1), 
	#endif
	#ifdef PID_COEFF_2
		PIDcontroller(PID_COEFF_2), 
	#endif
	#ifdef PID_COEFF_3
		PIDcontroller(PID_COEFF_3), 
	#endif
	#ifdef PID_COEFF_4
		PIDcontroller(PID_COEFF_4), 
	#endif
};

DiscretePulseFrequency signal[] = {
	#ifdef PID_PIN_1 
		DiscretePulseFrequency(PID_PIN_1),
	#endif
	#ifdef PID_PIN_2 
		DiscretePulseFrequency(PID_PIN_2),
	#endif
	#ifdef PID_PIN_3 
		DiscretePulseFrequency(PID_PIN_3),
	#endif
	#ifdef PID_PIN_4 
		DiscretePulseFrequency(PID_PIN_4),
	#endif
};

KalmanFilter1D filter[] = {
	#ifdef FILT_COEFF_1 
		KalmanFilter1D(FILT_COEFF_1),
	#endif
	#ifdef FILT_COEFF_2
		KalmanFilter1D(FILT_COEFF_2),
	#endif
	#ifdef FILT_COEFF_3
		KalmanFilter1D(FILT_COEFF_3),
	#endif
	#ifdef FILT_COEFF_4
		KalmanFilter1D(FILT_COEFF_4),
	#endif
};

//----------------- PID CONTROLLER FUNCTIONS

void setupControllers( float temp_av[], char stype[] ) {
	// set state
	CN391_setup(stype);

	PIDcontroller::initTimer();
	delay(1000); 						// add enough delay to ensure board is on

	// Calculate calibration average
	for( uint8_t ch = 0; ch < N_ENABLED; ch += 1 ) {
		temp_av[ch] = 0;				// must initialize to zero
	}
	
	for( uint8_t i = 0; i < N_AVERAGE; i += 1) {
		float tcTemp[NUM_PORT]; 
		CN391_getThermocoupleTemps(tcTemp);
	
		for( uint8_t ch = 0; ch < N_ENABLED; ch += 1 ) {
			temp_av[ch] += tcTemp[ch] / float(N_AVERAGE);
		}
	}

	// set filter states	
	for( uint8_t ch = 0; ch < N_ENABLED; ch += 1 ) {
		controller[ch].setState( temp_av[ch] );		// initalized to normalized average
		signal[ch].setup();
	}
}

void setupFilters( float temp_av[] ) {
	for( uint8_t ch = 0; ch < N_ENABLED; ch += 1 ) {
		filter[ch].setState( temp_av[ch] ); 		// initialize filters to current state
	}
}

void updateControllers( float target[], float measure[] ) {
	// update state
	PIDcontroller::updateTimeStep();

	// controller
	for( uint8_t ch = 0; ch < N_ENABLED; ch += 1 ) {
		controller[ch].update( target[ch], measure[ch] );
		signal[ch].update( controller[ch].output() ); 		// send Pulse Frequency modulated signal
	}
}

void updateOutputFilters( float measure[] ) {
	// filter raw values for serial data
	for( uint8_t ch = 0; ch < N_ENABLED; ch += 1 ) {
		filter[ch].update( measure[ch] );
	}
}


