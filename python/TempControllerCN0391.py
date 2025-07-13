"""
TempControllerCN0391
--------------------
This module is a self-contained wrapper for serial commands needed to control all of the parameters
of a temperature controller built around the CN0391 temperature shield. 
"""

# Global libraries
import time
# local file
import ArduinoSerial

#--- CONSTANTS ---
_DEFAULT_BAUD = 9600

#=========================================================================================

class TempControllerCN0391:

	def __init__(self, port, baud_rate=_DEFAULT_BAUD):
		self.serial = ArduinoSerial.SerialCommunication(port, baud_rate)
	
	# ---- setup -----
	
	# See: `arduino.ino`   -> setup()
    #      `SerialCom.ino` -> readSensorTypes()
	
	def _setup(self, cmd):
		# delay to initialize device
		self.serial.flush()
		time.sleep(2) 
		
		# check and send response
		while True:
			data = self.serial.read_serial()
			print(data)
			
			if data == 'WAITING-TYPES': # COMMAND MUST MATCH ARDUINO OUTPUT
				reply = self.setter(cmd)       # send sensor types
				print(reply)
			elif data  == "CALIBRATED": # COMMAND MUST MATCH ARDUINO OUTPUT
				self.flush()
				break
	
	def setup(self, type1=None, type2=None, type3=None, type4=None):
		# check inputs
		if type1 and type2 and type3 and type4:	
			cmd = str(type1) + str(type2) + str(type3) + str(type4)
			self._setup(cmd)	# Custom types
		else:
			self._setup('0')	# Default types
	
	# ----- wrappers -----
	def close(self):
		self.serial.close()
	
	def flush(self):
		self.serial.flush()
	
	def is_active(self):
		return self.serial.is_active()
	
	# ----- functions -----
		# private
	def getter(self, cmd, out=None):
		self.serial.write_serial(cmd)
		return self.serial.read_data(out)
	
	def setter(self, cmd):
		self.serial.write_serial(cmd)
		return self.serial.read_serial()	# recognize message is received
	
		# sensor
	def get_filter(self):
		cmd = "0"
		return self.getter(cmd, out='param')
	
	def get_raw(self):
		cmd = "1"
		return self.getter(cmd, out='param')
	
	def get_target(self):
		cmd = "2"
		return self.getter(cmd, out='param')
	
		# target
	def set_target(self, ch, target): 
		cmd = "3," + str(ch) + "," + str(target) 
		self.setter(cmd) 
	
	def set_target_all(self, targ1, targ2, targ3, targ4): 
		cmd = "3,4," + str(targ1) + "," + str(targ2) + "," + str(targ3) + "," + str(targ4)
		self.setter(cmd)
	
		# PID controller
	def get_pid(self, ch):
		cmd = "4," + str(ch)
		return self.getter(cmd, out='param')
	
	def set_pid(self, ch, kp, ki, kd):
		cmd = "5," + str(ch) + "," + str(kp) + "," + str(ki) + "," + str(kd)
		self.setter(cmd)
	
			# input limits
	def get_in_limit(self, ch):
		cmd = "6," + str(ch)
		return self.getter(cmd, out='param')
	
	def set_in_limit(self, ch, imax, imin):
		cmd = "7," + str(ch) + "," + str(imax) + "," + str(imin)
		self.setter(cmd)
	
		# Filters
			# alpha-beta
	def get_ab_filter(self, ch):
		cmd = "8," + str(ch)
		return self.getter(cmd, out='param')
	
	def set_ab_filter(self, ch, alpha, beta):
		cmd = "9," + str(ch) + "," + str(alpha) + "," + str(beta)
		self.setter(cmd)
	
			# kalman
	def get_k_filter(self, ch):
		cmd = "10," + str(ch)
		return self.getter(cmd, out='param')
	
	def set_k_filter(self, ch, error, noise):
		cmd = "11," + str(ch) + "," + str(error) + "," + str(noise)
		self.setter(cmd)
	
	def set_k_filter_state(self, ch, value):
		cmd = "12," + str(ch) + "," + str(value)
		self.setter(cmd)
	
		# Sensors
	def get_sensor_type(self): 	# NOTE: returns characters
		cmd = "13"
		return self.getter(cmd, out='str_arr')[1:]
	
		#Enable / Disable controllers
	def set_enable(self, ch):
		cmd = "14," + str(ch)
		self.setter(cmd)
	
	def set_enable_all(self): 
		cmd = "14,4"
		self.setter(cmd)
	
	def set_disable(self, ch):
		cmd = "15," + str(ch)
		self.setter(cmd)
	
	def set_disable_all(self): 
		cmd = "15,4"
		self.setter(cmd)
	
	def get_enable(self):
		cmd = "16"
		return self.getter(cmd, out='param')
	
		# timers
	def get_timer(self):
		cmd = "17"
		return self.getter(cmd, out='param')
	
	def get_timeout(self):
		cmd = "18"
		return self.getter(cmd, out='param')
	
	def set_timeout(self, ch, time):
		cmd = "19," + str(ch) + "," + str(time)
		self.setter(cmd)
	
	def set_timeout_inf(self, ch):
		cmd = "19," + str(ch) + ",-1"	# send value for infinite time. See `Commands.h`
		self.setter(cmd)

"""
Must document following format:

"example_func.py"
Multiplies two numbers and returns the result.

Args:
    a (int): The first number.
    b (int): The second number.

Returns:
    int: The product of a and b.

"""

