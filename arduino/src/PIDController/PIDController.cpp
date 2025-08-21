#include <Arduino.h>
#include "PIDController.h"

//==== Constants ====

// Controller output limits [must match limits of signal generator]
constexpr float OUT_MAX = 1;
constexpr float OUT_MIN = 0;
// Minimum difference between input limits. Prevents zero divison.
constexpr float IN_DIFF_MIN = 1; 


//==== Static functions ====

static float PIDcontroller::time_last = 0;
static float PIDcontroller::dt = 1;

// global timer
static void PIDcontroller::initTimer() {
	time_last = millis();
}

static void PIDcontroller::updateTimeStep() {
	uint32_t time_now = millis();
	dt = (time_now - time_last)*1e-3;
	time_last = time_now;
}

static float PIDcontroller::getTimeStep() {
	return dt;
}


//==== Constructors ====

PIDcontroller::PIDcontroller(
		float _kp   , float _ki   , float _kd, 
		float _imax,  float _imin, 
		float _alpha, float _beta ) 
{
	// pid
	gain_p = _kp;
	gain_i = _ki;
	gain_d = _kd;
	
	// filter
	alpha = LIMIT(_alpha);
	beta = LIMIT(_beta);
	
	// Initialize
	setInputLimits(_imax, _imin);
	setState(0);
}


//==== functions ====

void PIDcontroller::setState(float input) {
	_output = 0;
	integral = 0;
	dx_dt = 0;
	xvar = normalize(input);
}

void PIDcontroller::update( float target, float measure ) {
	// normalize
	target = normalize(target);
	measure = normalize(measure);

	// filter
	updateFilter(measure);

	// Sum components
	float error = target - filter();

	float integral_new = integral + gain_i*error*dt;

	float output = gain_p*error + integral_new - gain_d*deriv(); // ignore target derivative

	// update output
	if( output > OUT_MAX ) {   // saturate output 
		output = OUT_MAX;

	} else if ( output < OUT_MIN ) {
		output = OUT_MIN;

	} else {		           		// Prevent integral windup
		integral = integral_new; 	// out_min < output < out_max
	}
	_output = output;				// store output
}

void PIDcontroller::updateFilter(float input) {
	// smoother
	float dx = input - xvar;
	xvar += dx_dt*dt + alpha*dx;
	// derivative
	dx_dt += beta * dx/dt;
}


//==== Input Limits ====

// NOTE: cannot constrain() normalized values as these are satured at the PID controller output

float PIDcontroller::normalize(float input) {
	return input*in_scale + in_offset; // Beware sign of offset
}

void PIDcontroller::setInputLimits(float imax, float imin) {
	// prevent zero division
	if( abs(imax - imin) < IN_DIFF_MIN ) {	// prevent sign errors
		imax = IN_DIFF_MIN;      // imax = IN_DIFF_MIN
		imin = 0;                // imin = 0
	}
	
	in_scale = 1.0/(imax - imin); // This code is suceptible to run-time errors if ram is full.
	in_offset = -imin * in_scale;
}

void PIDcontroller::getInputLimits( float output[] ) {
	// Zero division prevented in setter
	float inv = 1 / in_scale;
	float imin = -in_offset * inv;
	float imax = inv + imin;	// negative included with imin
	// return limits
	output[0] = imax;
	output[1] = imin;
}


//==== Simple getters ====

float PIDcontroller::output() {
	return _output;
}

float PIDcontroller::deriv() {
	return dx_dt;
}

float PIDcontroller::filter() {
	return xvar;
}


//==== PID Gains ===

void PIDcontroller::setPIDGains(float kp, float ki, float kd) {
	gain_p = kp;
	gain_i = ki;
	gain_d = kd;
}

void PIDcontroller::getPIDGains( float output[] ) {
	output[0] = gain_p;
	output[1] = gain_i;
	output[2] = gain_d;
}


//==== Filter Gains ====

void PIDcontroller::getFilterGains( float output[] ) {
	output[0] = alpha;
	output[1] = beta;
}

void PIDcontroller::setFilterGains(float _alpha, float _beta) {
	alpha = LIMIT(_alpha);
	beta = LIMIT(_beta);
}

void PIDcontroller::setFilterGains(float _alpha) {
	alpha = LIMIT(_alpha);
	beta = LIMIT(0.5 * alpha * alpha);	// constant damping ratio
}


