#ifndef Constants_h
#define Constants_h

//======================================

/* Default PID coefficients: */
//                             PID       |  INPUT (C) |  FILTER
//                      kp,   ki,    kd  | imax, imin | alpha, beta
#define PID_COEFF_1	    0.0,  0.0,   0.0,  5.0,  0.0,   0.0,   0.0
#define PID_COEFF_2     0.0,  0.0,   0.0,  10.0,  0.0,   0.0,   0.0
#define PID_COEFF_3     0.0,  0.0,   0.0,  20.0,  0.0,   0.0,   0.0
#define PID_COEFF_4     0.0,  0.0,   0.0,  30.0,  0.0,   0.0,   0.0

/* PID output pins: */
//					diginal pin
#define PID_PIN_1		2
#define PID_PIN_2		3
#define PID_PIN_3		4
#define PID_PIN_4		5

// NOTE: Pins 10, 11, 12, and 13, are for SPI. Reserved. 

/* Default Output signal coefficients: */
// 						error, noise
#define FILT_COEFF_1	0.0,   0.0
#define FILT_COEFF_2	0.0,   0.0
#define FILT_COEFF_3	0.0,   0.0
#define FILT_COEFF_4	0.0,   0.0

/* Default PID target temperatures: [required] */
//                      ch0 | ch1 | ch2 | ch3
#define PID_TARGET      0,    0,    0,    0

/* Default Sensor types */
// 			Options: 	T, J, K, E, S, R, N, B
#define SENSOR_TYPE 	'N',  'N',  'N',  'N'

/* NOTE: 
	- Type 'N' sensors are thermocuple wires. 
	- NUM_PORT defined in "BasicCN0391.h"
	- Reduce buffer size as much as possible to avoid silent out-of-memory errors.
	  Errors are known to occur with BUFFER_SIZE>=40 
*/
//======================================

// Sensors:
constexpr int 	   N_AVERAGE   = 10;     // Number of measurements to average when initializing filters
constexpr uint32_t PULSE_WIDTH = 100000; // Smallest pulse width of frequency-pulse-modulation (us)

// Serial communication:
constexpr int 	BAUD_RATE   = 9600;   // speed of serial communication. MUST match value in "main.py"
constexpr int 	BUFFER_SIZE = 30;     // maximum allowed characters in serial input buffer
constexpr int 	INPUTS_MAX  = 6;      // <command> <channel> <param1> <param2> <param3> <param4> 
constexpr int 	DECIMAL_MAX = 2;			// Maximum number of decimals for serial OUTPUTS; More decimals are rounded.

	// Strings
#define DELIM_CHAR     ","	     // character used to split serial commands; Wrap with ""
#define END_LINE_CHAR  '\n'      // character used to detect the end of a string captured via serial

#endif

