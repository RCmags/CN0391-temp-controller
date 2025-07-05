#ifndef Commands_h
#define Commands_h

enum CHANNEL {
	CH0,    // 0
	CH1,    // 1
	CH2,    // 2
	CH3,    // 3
	CH_ALL  // 4
};

enum FUNCTION {
	GET_FILTER,         // 0	// Sensor output
	GET_RAW,            // 1
	GET_TARGET,         // 2	// Target values
	SET_TARGET,         // 3
	GET_PID,            // 4	// PID
	SET_PID,            // 5
	GET_IN_LIMIT,       // 6	// Input Limits
	SET_IN_LIMIT,       // 7
	GET_OUT_LIMIT,      // 8	// Output Limits >>[must be 0, 1]
	SET_OUT_LIMIT,      // 9
	GET_AB_FILTER,      // 10	// Alpha-Beta Filter
	SET_AB_FILTER,      // 11
	GET_K_FILTER,       // 12	// Kalman filter
	SET_K_FILTER,       // 13
	SET_K_FILTER_STATE, // 14
	GET_SENSOR_TYPE,    // 15	// sensor
	SET_ENABLE,         // 16	// enable/disable PID controllers
	SET_DISABLE,        // 17
	GET_ENABLE,         // 18
	GET_TIMER,          // 19   // timer
	GET_TIMEOUT,        // 20
	SET_TIMEOUT         // 21
};

// need python script to convert string to number for serial commands. 

#endif
