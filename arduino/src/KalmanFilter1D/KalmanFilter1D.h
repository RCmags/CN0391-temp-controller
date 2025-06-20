// This is a modified version of the library by Denyssene:
// See: https://github.com/denyssene/SimpleKalmanFilter

#ifndef KalmanFilter1D_h
#define KalmanFilter1D_h

class KalmanFilter1D {
	private:
		// parameters
		float var_measure;
		float qval;

		// variables
		float x;
		float var = 1;

	public:
		KalmanFilter1D(float, float);

		void setState(float);
		void update(float);
		float value();
};

#endif
