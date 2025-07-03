#ifndef Commands_h
#define Commands_h

enum CHANNEL {
	CH0,	// 0
	CH1,	// 1
	CH2,	// 2
	CH3,	// 3
	CH_ALL	// 4
};

enum FUNCTION {
	// getters
		// output
	GET_FILTER,			// 0
	GET_RAW,			// 1
	GET_TARGET,			// 2
		// pid
	GET_PID,			// 3
	GET_IN_LIMIT,		// 4
	GET_OUT_LIMIT,		// 5
		// filters
	GET_AB_FILTER,		// 6
	GET_K_FILTER,		// 7
	// setters
		// targets
	SET_TARGET,			// 8
		// pid
	SET_PID,			// 9
	SET_IN_LIMIT,		// 10
	SET_OUT_LIMIT,		// 11
		// filter
	SET_AB_FILTER,		// 12
	SET_K_FILTER,		// 13
	SET_K_FILTER_STATE,	// 14
		// sensor
	GET_SENSOR_TYPE		// 15
};

// need python script to convert string to number for serial commands. 

#endif
