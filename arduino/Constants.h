#ifndef Constants_h
#define Constants_h

/* 	          Instructions 
----------------------------------------
Enable "PID_COEFF", "PID_PIN", and "FILT_COEFF" for each thermocouple connected to the CN0391 
sensor shield. "N_ENABLED" Must match the number of enabled PID_COEFF parameters.
*/
//======================================

// Number of channels from which data is received, and for which PID controllers are enabled.
constexpr int 	N_ENABLED 	= 2;

/* Default PID coefficients: [required] */
//                                 PID       |   OUTPUT   |   INPUT    |  FILTER
//                          kp,   ki,    kd  | omax, omin | imax, imin | alpha, beta
#define PID_COEFF_1_DEF	    15,   0.25,  15,   1,    0,     100,  0,     0.5,   0.05
#define PID_COEFF_2_DEF     1,    0.010, 20,   1,    0,     100,  0,     0.5,   0.05
//#define PID_COEFF_3_DEF     15,   0.25,  15,   1,    0,     100,  0,     0.5,   0.05
//#define PID_COEFF_4_DEF     15,   0.25,  15,   1,    0,     100,  0,     0.5,   0.05

/* PID output pins: */
//					diginal pin
#define PID_PIN_1		2
#define PID_PIN_2		3
//#define PID_PIN_3		4
//#define PID_PIN_4		5

/* Output signal coefficients: */
// 						error, noise
#define FILT_COEFF_1	1, 0.1
#define FILT_COEFF_2	1, 0.1
//#define FILT_COEFF_3	1, 0.1
//#define FILT_COEFF_4	1, 0.1

/* Sensor types */
// 			Options: 	T, J, K, E, S, R, N, B
#define SENSOR_TYPE 	'N', 'N', 'N', 'N'

// NOTE: Type 'N' sensors are thermocouple wires. 

//======================================

// Normalize temperature measurements for PID controller:
constexpr float TEMP_MAX 	= 100; 	// Degrees C 
constexpr float TEMP_MIN 	= 0;	// Degrees C
constexpr int 	N_AVERAGE 	= 10;	// Number of measurements to average when initializing filters

// Serial communication:
constexpr int 	BAUD_RATE	= 9600;	// speed of serial communication. MUST match value in "main.py"
constexpr int 	N_CHAR  	= 40;	// maximum allowed characters in serial input buffer

// NOTE: NUM_PORT defined in "BasicCN0391.h"

#endif

