#ifndef PIDController_h
#define PIDController_h

// Macro to restrict constant values
#define LIMIT(X) X < 0 ? 0 : X > 1 ? 1 : X

class PIDcontroller {
  private:
   	// Parameters
    float gain_p;
    float gain_i;
    float gain_d;
    float out_max;
    float out_min;
    float alpha;
    float beta;

	// variables
		// timer
	static float time_last;
	static float dt;

		// pid state
	float _output;
	float integral; 
	float dx_dt;
	float xvar;

	//==== functions ====
	void updateFilter(float);

  public:
  	// constructors
  	PIDcontroller() {} // manually set coefficients
  	
    PIDcontroller(float, float, float, 
                  float, float, 
                  float, float);
	
	// setters
	void setPIDGains(float, float, float);
	void setLimits(float, float);
	void setFilterGains(float, float);
	void setFilterGains(float);

	// global timer
	static void initTimer();
	static void updateTimeStep();

	// controller functions
    void setState(float);
    void update(float, float);
    
    // getter functions
	float output();
	float filter();
	float deriv();
};

#endif
