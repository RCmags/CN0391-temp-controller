import ArduinoSerial

class TempControllerCN0391:

	def __init__(self, port, baud_rate=9600):
		self.serial = ArduinoSerial.SerialCommunication(port, baud_rate)
		
	# ----- helpers -----
	def setup(self, sensor1, sensor2, sensor3, sensor4) {
		time.sleep(2) 				# delay to initialize device
		output = self._set_sensor_type(sensor1, sensor2, sensor3, sensor4)
		print( output["func"] )
	}
	
	def close(self):
		self.serial.close()
	
	def flush(self):
		self.serial.flush()
	
	def get_reply(self):
		self.serial.read_data()
	
	# ----- functions -----
		# private
	def _getter(self, func):
		self.serial.write_data(func)
		return self.get_reply()
	
	def _setter(self, func):
		self.serial.write_data(func)
	
	#---
	
		# sensor
	def get_filter(self):
		cmd = "0"
		return self._getter(cmd)["param"]
	
	def get_raw(self):
		cmd = "1"
		return self._getter(cmd)["param"]
	
	def get_target(self):
		cmd = "2"
		return self._getter(cmd)["param"]
	
	#---
	
		# target
	def set_target(self, ch, target):
		cmd = "3," + str(ch) + "," + str(target)
		return self._setter(cmd)
	
	def set_target(self, targ1, targ2, targ3, targ4):
		cmd = "3,-1," + str(targ1) + "," + str(targ2) + "," + str(targ3) + "," + str(targ4)
		return self._setter(cmd)
	
	#---
	
		# PID controller
	def get_pid(self, ch):
		cmd = "4," + str(ch)
		return self._getter(cmd)["param"]
	
	def set_pid(self, ch, ki, kp, kd):
		cmd = "5," + str(ch) + "," + str(ki) + "," + str(kd) + "," + str(kp)
		return self._setter(cmd)
	
			# input limits
	def get_in_limit(self, ch):
		cmd = "6," + str(ch)
		return self._getter(cmd)["param"]
	
	def set_in_limit(self, ch, imax, imin):
		cmd = "7," + str(ch) + "," + str(imax) + "," + str(imin)
		return self._setter(cmd)
	
	#---
	
		# Filters
			# alpha-beta
	def get_ab_filter(self, ch):
		cmd = "8," + str(ch)
		return self._getter(cmd)["param"]
	
	def set_ab_filter(self, ch, alpha, beta):
		cmd = "9," + str(ch) + "," + str(alpha) + "," + str(beta)
		return self._setter(cmd)
	
			# kalman
	def get_k_filter(self, ch):
		cmd = "10"
		return self._getter(cmd)["param"]
	
	def set_k_filter(self, ch, error, noise):
		cmd = "11," + str(ch) + "," + str(error) + "," + str(noise)
		return self._setter(cmd)
	
	def set_k_filter_state(self, ch, value):
		cmd = "12," + str(ch) + "," + str(value)
		return self._setter(cmd)
	
	#---
	
		# Sensors
	def get_sensor_type(self): 				# NOTE: returns characters
		cmd = "13"
		self.serial.write_data(cmd)
		string = self.serial.read_serial_string()
		return string.split(",")
	
	def _set_sensor_type(self, s1, s2, s3, s4):
		cmd = str(s1) + str(s2) + str(s3) + str(s4)
		return self._setter(cmd)	# gets data
	
	def set_sensor_type_default(self):
		cmd = "0"
		return self._setter(cmd)
	
	#---
	
		#Enable / Disable controllers
	def set_enable(self, ch):
		cmd = "14," + str(ch)
		return self._setter(cmd)
	
	def set_enable(self):
		cmd = "14,-1"
		return self._setter(cmd)
	
	def set_disable(self, ch):
		cmd = "15," + str(ch)
		return self._setter(cmd)
	
	def set_disable(self):
		cmd = "15,-1"
		return self._setter(cmd)
	
	def get_enable(self):
		cmd = "16"
		return self._getter(cmd)["param"]
	
	#---
	
		# timers
	def get_timer(self):
		cmd = "17"
		return self._getter(cmd)["param"]
	
	def get_timeout(self):
		cmd = "18"
		return self._getter(cmd)["param"]
	
	def set_timeout(self, ch, time):
		cmd = "18," + str(ch) + "," + str(time)
		return self._setter(cmd)
	
	def set_timeout_inf(self, ch):
		cmd = "18," + str(ch) + ",-1"
		return self._setter(cmd)



