# global libraries
import time

# local file
import ArduinoSerial

#--- CONSTANTS ---
_DEFAULT_BAUD = 9600
_END_LINE_CHAR = '\n'

#=========================================================================================

class TempControllerCN0391:

	def __init__(self, port, baud_rate=_DEFAULT_BAUD):
		self.serial = ArduinoSerial.SerialCommunication(port, baud_rate)
	
	# ---- setup -----
	
	def _setup(self, cmd):
		# delay to initialize device
		time.sleep(2) 
		
		# check and send response
		while True:
			data = self.serial.read_data()["str"]
			print(data)
			
			if data == 'WAITING-TYPES':
				self._setter(cmd)		# send sensor types
			elif data  == "CALIBRATED":
				break
	
	
	def setup(self, type1=None, type2=None, type3=None, type4=None):
		# check inputs
		if type1 and type2 and type3 and type4:	
			cmd = str(type1) + str(type2) + str(type3) + str(type4)
			self._setup(cmd)	# Custom types
		else:
			self._setup('0')	# Default types
	
	
	# ----- helpers -----
	def close(self):
		self.serial.close()
	
	def flush(self):
		self.serial.flush()

	# ----- functions -----
		# private
	def _getter(self, cmd):
		self.serial.write_data(cmd)
		return self.serial.read_data()
	
	def _setter(self, cmd):
		cmd = f'{cmd}{_END_LINE_CHAR}' # add end-of-line to avoid blocking
		self.serial.write_data(cmd)
	
	
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
	
	
	# Note: cannot overload functions
	
		# target
	def set_target(self, ch, target): 
		cmd = "3," + str(ch) + "," + str(target) 
		self._setter(cmd) 
	
	def set_target_all(self, targ1, targ2, targ3, targ4): 
		cmd = "3,4," + str(targ1) + "," + str(targ2) + "," + str(targ3) + "," + str(targ4)
		self._setter(cmd)
	
	
		# PID controller
	def get_pid(self, ch):
		cmd = "4," + str(ch)
		return self._getter(cmd)["param"]
	
	def set_pid(self, ch, ki, kp, kd):
		cmd = "5," + str(ch) + "," + str(ki) + "," + str(kd) + "," + str(kp)
		self._setter(cmd)
	
			# input limits
	def get_in_limit(self, ch):
		cmd = "6," + str(ch)
		return self._getter(cmd)["param"]
	
	def set_in_limit(self, ch, imax, imin):
		cmd = "7," + str(ch) + "," + str(imax) + "," + str(imin)
		self._setter(cmd)
	
	
		# Filters
			# alpha-beta
	def get_ab_filter(self, ch):
		cmd = "8," + str(ch)
		return self._getter(cmd)["param"]
	
	def set_ab_filter(self, ch, alpha, beta):
		cmd = "9," + str(ch) + "," + str(alpha) + "," + str(beta)
		self._setter(cmd)
	
			# kalman
	def get_k_filter(self, ch):
		cmd = "10," + str(ch)
		return self._getter(cmd)["param"]
	
	def set_k_filter(self, ch, error, noise):
		cmd = "11," + str(ch) + "," + str(error) + "," + str(noise)
		self._setter(cmd)
	
	def set_k_filter_state(self, ch, value):
		cmd = "12," + str(ch) + "," + str(value)
		self._setter(cmd)
	

		# Sensors
	def get_sensor_type(self): 				# NOTE: returns characters
		self.serial.write_data("13")
		string = self.serial.read_serial_string()
		return string.split(",")[1:]		# -> Modify general function to spit out characters?
	
	
		#Enable / Disable controllers
	def set_enable(self, ch):
		cmd = "14," + str(ch)
		self._setter(cmd)
	
	def set_enable_all(self): 
		cmd = "14,4"
		self._setter(cmd)
	
	def set_disable(self, ch):
		cmd = "15," + str(ch)
		self._setter(cmd)
	
	def set_disable_all(self): 
		cmd = "15,4"
		self._setter(cmd)
	
	def get_enable(self):
		cmd = "16"
		return self._getter(cmd)["param"]
	
	
		# timers
	def get_timer(self):
		cmd = "17"
		return self._getter(cmd)["param"]
	
	def get_timeout(self):
		cmd = "18"
		return self._getter(cmd)["param"]
	
	def set_timeout(self, ch, time):
		cmd = "19," + str(ch) + "," + str(time)
		self._setter(cmd)
	
	def set_timeout_inf(self, ch):
		cmd = "19," + str(ch) + ",-1"
		self._setter(cmd)

# NOTE: functions may hang due to errors in the arduino code. Check errors on that end!

