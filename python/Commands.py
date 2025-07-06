"""
COMMANDS: This is a manual conversion of the enums located in /arduino/Commands.h
"""

#---- Sensor output
def get_filter():
	return "0"

def get_raw():
	return "1"

def get_target():
	return "2"

#---- Target values
def set_target(ch, target):
	return "3," + str(ch) + "," + str(target)

def set_target(targ1, targ2, targ3, targ4):
	return "3,-1," + str(targ1) + "," + str(targ2) + "," + str(targ3) + "," + str(targ4)

#---- PID controller
def get_pid(ch):
	return "4," + str(ch)

def set_pid(ch, ki, kp, kd):
	return "5," + str(ch) + "," + str(ki) + "," + str(kd) + "," + str(kp)

	# Input limits
def get_in_limit(ch):
	return "6," + str(ch)

def set_in_limit(ch, imax, imin):
	return "7," + str(ch) + "," + str(imax) + "," + str(imin)

#---- Filters:
	# Alpha-beta
def get_ab_filter(ch):
	return "8," + str(ch)

def set_ab_filter(ch, alpha, beta):
	return "9," + str(ch) + "," + str(alpha) + "," + str(beta)

	# Kalman
def get_k_filter(ch):
	return "10"

def set_k_filter(ch, error, noise):
	return "11," + str(ch) + "," + str(error) + "," + str(noise)

def set_k_filter_state(ch, value):
	return "12," + str(ch) + "," + str(value)

#---- Sensors
def get_sensor_type():
	return "13"
	
def set_sensor_type(s1, s2, s3, s4):
	return str(s1) + str(s2) + str(s3) + str(s4)
	
def set_sensor_type_default():
	return "0"

#---- Enable / Disable controllers
def set_enable(ch):
	return "14," + str(ch)

def set_enable():
	return "14,-1"

def set_disable(ch):
	return "15," + str(ch)

def set_disable():
	return "15,-1"

def get_enable():
	return "16"

#---- Timers
def get_timer():
	return "17"

def get_timeout():
	return "18"

def set_timeout(ch, time):
	return "18," + str(ch) + "," + str(time)

