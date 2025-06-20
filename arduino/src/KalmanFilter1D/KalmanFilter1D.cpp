#include <Arduino.h>
#include "KalmanFilter1D.h"

// constructor
KalmanFilter1D::KalmanFilter1D( float _error, float _qval ) {
	var_measure = _error*_error;
	qval = _qval;
}

// filter functions
void KalmanFilter1D::setState(float input) {
	x = input;
}

void KalmanFilter1D::update(float xin) {
	float gain = var/(var + var_measure);
	float dx = (xin - x);
	float dxq = dx*qval;
	var = (1-gain)*var + dxq*dxq;			// update variance
	x += gain*dx;							// update estimate
}

float KalmanFilter1D::value() {				// get filter output
	return x;
}
