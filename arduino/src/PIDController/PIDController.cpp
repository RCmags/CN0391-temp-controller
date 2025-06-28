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
	gain_p = _kp;
	gain_i = _ki;
	gain_d = _kd;
	out_max = _omax;
	out_min = _omin;
	in_max = _imax;
	in_min = _imin;
	alpha = LIMIT(_alpha);
	beta = LIMIT(_beta);
}


//==== public functions ====

void PIDcontroller::updateFilter(float input) {
	// smoother
	float dx = input - xvar;
	xvar += dx_dt*dt + alpha*dx;
	// derivative
	dx_dt += beta * dx/dt;
}


// parameter setters
void PIDcontroller::setPIDGains(float kp, float ki, float kd) {
	gain_p = kp;
	gain_i = ki;
	gain_d = kd;
}

void PIDcontroller::setOutputLimits(float omax, float omin) {
	if( omax > omin ) {
		out_max = omax;
		out_min = omin;
	}
}

void PIDcontroller::setInputLimits(float imax, float imin) {
	if( in_max > in_min ) {
		in_max = imax;
		in_min = imin;
	}
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
  xvar = input;
}

void PIDcontroller::update( float target, float measure ) {
	// filter
	updateFilter(measure);    	
  	// Sum components
    float error = target - xvar;
  
    float integral_new = integral + gain_i*error*dt;
    
    float output = gain_p*error + integral_new - gain_d*dx_dt; // ignore target derivative

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

