#include <Arduino.h>
#include "KalmanFilter1D.h"

// constructor
KalmanFilter1D::KalmanFilter1D( float _error, float _qval ) {
	setGains(_error, _qval);
}


// setters
void KalmanFilter1D::setGains( float _error, float _qval) {
	if( _error > 0 && _qval > 0 ) {
		var_measure = _error*_error;
		qval = _qval;
	} else {
		var_measure = 1; // default values
		qval = 0;
	}
}

void KalmanFilter1D::setState( float input ) {
	x = input;
}

// filter functions
void KalmanFilter1D::update( float xin ) {
	float gain = var/(var + var_measure);
	float dx = (xin - x);
	float dxq = dx*qval;
	var = (1-gain)*var + dxq*dxq;			// update variance
	x += gain*dx;							// update estimate
}


// getters
float KalmanFilter1D::value() {				// get filter output
	return x;
}

void KalmanFilter1D::getGains( float output[] ) {
	// reverse conversion
	output[0] = sqrt(var_measure);
	output[1] = qval;
}

