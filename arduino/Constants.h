#ifndef Constants_h
#define Constants_h

/* 	          Instructions 
----------------------------------------
Enable "PID_COEFF", "PID_PIN", and "FILT_COEFF" for each thermocouple connected to the CN0391 
sensor shield. "N_ENABLED" Must match the number of enabled PID_COEFF parameters.
*/
//======================================

// NOTE: NUM_PORT defined in "BasicCN0391.h"

/* Default PID coefficients: [required] */
//                             PID       |   OUTPUT   | INPUT (C)  |  FILTER
//                      kp,   ki,    kd  | omax, omin | imax, imin | alpha, beta
#define PID_COEFF_1	    15,   0.25,  20,   1,    0,     100,  0,     0.5,   0.05
#define PID_COEFF_2     0.75, 0.010, 8,    1,    0,     100,  0,     0.5,   0.05
#define PID_COEFF_3     0.0,  0.0,   0.0,  0.0,  0.0,   5.0,  0.0,   0.0,   0.0
#define PID_COEFF_4     0.0,  0.0,   0.0,  0.0,  0.0,   5.0,  0.0,   0.0,   0.0

/* PID output pins: */
//					diginal pin
#define PID_PIN_1		2
#define PID_PIN_2		3
#define PID_PIN_3		4
#define PID_PIN_4		5

// NOTE: Pins 10, 11, 12, and 13, are for SPI. Reserved. 

/* Default Output signal coefficients: */
// 						error, noise
#define FILT_COEFF_1	1.0,   0.1
#define FILT_COEFF_2	1.1,   0.2
#define FILT_COEFF_3	0.0,   0.0
#define FILT_COEFF_4	0.0,   0.0


/* Default PID target temperatures: [required] */
//                      ch0 | ch1 | ch2 | ch3
#define PID_TARGET      0,    0,    0,    0

/* Default Sensor types */
// 			Options: 	T, J, K, E, S, R, N, B
#define SENSOR_TYPE 	'N',  'N',  'N',  'N'    // must change on the fly
//#define SENSOR_TYPE 	'K',  'J',  'N',  'B'    // must change on the fly

// NOTE: Type 'N' sensors are thermocouple wires. 
// ^ variables should be able to be changed. need to set sensor types. 
// ^ runtime breaks when channels have no sensor. Need to detect when sensor is missing. 

//======================================

constexpr int 	N_ENABLED 	= 4;	// Number of channels from which data is received and PID controllers are enabled.
constexpr int 	N_AVERAGE 	= 10;	// Number of measurements to average when initializing filters

// Serial communication:
constexpr int 	BAUD_RATE	 = 9600;	// speed of serial communication. MUST match value in "main.py"
constexpr int 	N_CHAR  	 = 40;	  	// maximum allowed characters in serial input buffer
constexpr int   INPUTS_MAX = 6;     	// <command> <channel> <param1> <param2> <param3> <param4> 

#endif

