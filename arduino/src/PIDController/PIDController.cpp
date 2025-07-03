#include <Arduino.h>
#include "PIDController.h"

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


//==== Constructors ====

PIDcontroller::PIDcontroller(
		float _kp   , float _ki   , float _kd, 
		float _omax , float _omin , float _imax, float _imin, 
		float _alpha, float _beta ) 
{
	// pid
	gain_p = _kp;
	gain_i = _ki;
	gain_d = _kd;
	
	// filter
	alpha = LIMIT(_alpha);
	beta = LIMIT(_beta);
	
	// limits 
	setOutputLimits(_omax, _omin);
	setInputLimits(_imax, _imin);
}


//==== functions ====

void PIDcontroller::updateFilter(float input) {
	// smoother
	float dx = input - xvar;
	xvar += dx_dt*dt + alpha*dx;
	// derivative
	dx_dt += beta * dx/dt;
}

float PIDcontroller::normalize(float input) {
	return input*in_scale - in_offset;
}

// parameter setters
void PIDcontroller::setInputLimits(float imax, float imin) {
	if( imax > imin ) {
		// pre-calculate constants
		in_scale = 1.0/(imax - imin);
		in_offset = imin * in_scale;
	} else {
		in_scale = 1;
		in_offset = 0;
	}
}

void PIDcontroller::setOutputLimits(float omax, float omin) {
	if( omax > omin ) {
		out_max = omax;
		out_min = omin;
	}
}

void PIDcontroller::setPIDGains(float kp, float ki, float kd) {
	gain_p = kp;
	gain_i = ki;
	gain_d = kd;
}

void PIDcontroller::setFilterGains(float _alpha, float _beta) {
	alpha = LIMIT(_alpha);
	beta = LIMIT(_beta);
}

void PIDcontroller::setFilterGains(float _alpha) {
	alpha = LIMIT(_alpha);
	beta = LIMIT(0.5 * alpha * alpha);	// constant damping ratio
}


// controller functions
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
    if( output > out_max ) {   // saturate output 
      output = out_max;

    } else if ( output < out_min ) {
      output = out_min;

    } else {		           	// Prevent integral windup
      integral = integral_new; 	// out_min < output < out_max
    }
    _output = output;			// store output
}


// getter functions
float PIDcontroller::output() {
	return _output;
}

float PIDcontroller::deriv() {
	return dx_dt;
}

float PIDcontroller::filter() {
	return xvar;
}


void PIDcontroller::getPIDGains( float output[] ) {
	output[0] = gain_p;
	output[1] = gain_i;
	output[2] = gain_d;
}

void PIDcontroller::getFilterGains( float output[] ) {
	output[0] = alpha;
	output[1] = beta;
}

void PIDcontroller::getOutputLimits( float output[] ) {
	output[0] = out_max;
	output[1] = out_min;
}

void PIDcontroller::getInputLimits( float output[] ) {
	// undo conversion
	float in_min = in_offset / in_scale;
	float in_max = in_min + 1.0/in_scale;
	// limits
	output[0] = in_max;
	output[1] = in_min;
}


