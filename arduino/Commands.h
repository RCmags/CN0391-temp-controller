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
	GET_AB_FILTER,      // 8	// Alpha-Beta Filter
	SET_AB_FILTER,      // 9
	GET_K_FILTER,       // 10	// Kalman filter
	SET_K_FILTER,       // 11
	SET_K_FILTER_STATE, // 12
	GET_SENSOR_TYPE,    // 13	// sensor
	SET_ENABLE,         // 14	// enable/disable PID controllers
	SET_DISABLE,        // 15
	GET_ENABLE,         // 16
	GET_TIMER,          // 17   // timer
	GET_TIMEOUT,        // 18
	SET_TIMEOUT         // 19
};

constexpr int TIME_INF = -1;   // Label to disable timeout

// need python script to convert string to number for serial commands. 

#endif
