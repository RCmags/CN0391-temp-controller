//----------------- ARRAYS OF CLASSES

// use preprocessor macros to program controllers in seperate file.

PIDcontroller controller[] = {
	#ifdef PID_COEFF_1_DEF
		PIDcontroller(PID_COEFF_1_DEF), 
	#endif
	#ifdef PID_COEFF_2_DEF
		PIDcontroller(PID_COEFF_2_DEF), 
	#endif
	#ifdef PID_COEFF_3_DEF
		PIDcontroller(PID_COEFF_3_DEF), 
	#endif
	#ifdef PID_COEFF_4_DEF
		PIDcontroller(PID_COEFF_4_DEF), 
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

float normalizeTemp( float input ) {
	constexpr float SLOPE = 1.0/(TEMP_MAX - TEMP_MIN);
	return (input - TEMP_MIN)*SLOPE;
}

void setupControllers( float temp_av[], char stype[] ) {
	// set state
	CN391_setup(stype);

	PIDcontroller::initTimer();
	delay(1000); 						// add enough delay to ensure board is on

	// Calculate calibration average
	for( int ch = 0; ch < N_ENABLED; ch += 1 ) {
		temp_av[ch] = 0;				// must initialize to zero
	}
	
	for( int i = 0; i < N_AVERAGE; i += 1) {
		float tcTemp[NUM_PORT]; 
		CN391_getThermocoupleTemps(tcTemp);
	
		for( int ch = 0; ch < N_ENABLED; ch += 1 ) {
			temp_av[ch] += tcTemp[ch] / float(N_AVERAGE);
		}
	}

	// set filter states	
	for( int ch = 0; ch < N_ENABLED; ch += 1 ) {
		//float temp_norm = normalizeTemp( temp_av[ch] );
		float temp_norm = temp_av[ch];
		
		controller[ch].setState( temp_norm );		// initalized to normalized average
		signal[ch].setup();
	}
}

void setupFilters( float temp_av[] ) {
	for( int ch = 0; ch < N_ENABLED; ch += 1 ) {
		filter[ch].setState( temp_av[ch] ); 		// initialize filters to current state
	}
}

void updateControllers( float target[], float measure[] ) {
	// update state
	PIDcontroller::updateTimeStep();
	CN391_getThermocoupleTemps(measure);

	// controller
	for( int ch = 0; ch < N_ENABLED; ch += 1 ) { 	
		//float meas_norm = normalizeTemp( measure[ch] );		// use normalized input
		//float targ_norm = normalizeTemp( target[ch] );
		float meas_norm = measure[ch];
		float targ_norm = target[ch];

		controller[ch].update( targ_norm, meas_norm );
		signal[ch].update( controller[ch].output() ); 		// send Pulse Frequency modulated signal
	}
}

void updateOutputFilters( float measure[] ) {
	// filter raw values for serial data
	for( int ch = 0; ch < N_ENABLED; ch += 1 ) {
		filter[ch].update( measure[ch] );
	}
}

