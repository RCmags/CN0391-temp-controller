"""
This module is a wrapper for the serial commands needed to adjust all of the parameters
of a temperature controller built with the CN0391 temperature shield and an Arduino Uno.
"""

# Note: Serial commands are specified in "Commands.h" within the Arduino sketch. 

# Global libraries
import time
import json

# local file
import ArduinoSerial

#--- CONSTANTS ---
_DEFAULT_BAUD = 9600

#===================================================================================================

class TempControllerCN0391:

	def __init__(self, port=None, baud_rate=_DEFAULT_BAUD, path=None):
		'''Initialize the temperature controller.
		
		Parameters
		----------
		port: str
			Serial port used by the Arduino to communicate with the computer. \
			Required if `path` is not provided. The Serial port changes with operating system:
		
		- On Linux and macOS, use '/dev/ttyACM0' or similar
		- On Windows, use 'COM3' or similar
		
		baud_rate: int
			Baud rate at which the Arduino is configured. Optional parameter that defaults to \
			hardcoded value if none is provided
		
		path : str
			Path to JSON file containing the controller parameters. Parameter is optional \
			and overrides `port` and `baud_rate`
		'''
		# initialize dictionary
		self.json_data = {}
		
		# json file is provided
		if type(path) == str:
			self.load_json_file(path)	# get json_data
			self.serial = ArduinoSerial.SerialCommunication( self.json_data["serial_port"], \
			                                                 self.json_data["baud_rate"] )
			# load data
			self.setup( *self.json_data["sensor_types"] )
			self.set_json_coefficients()
		
		# no json file
		elif path == None:
			self.serial = ArduinoSerial.SerialCommunication(port, baud_rate)
			# populate empty dict
			self.json_data = self._construct_json()
			self.json_data["serial_port"] = port
			self.json_data["baud_rate"] = baud_rate
	
	
	# ================ setup ================
	# See: `arduino.ino`   -> setup()
    #      `SerialCom.ino` -> readSensorTypes()
	
	def _setup(self, cmd):
		'''Private implementation of setup(). Captures and sends serial commands \
		to configure sensor types. Calibration is performed automatically by the Arduino. 
		
		Parameters
		----------
		cmd : string
			Serial command 
		'''
		# delay to initialize device
		self.serial.flush()
		time.sleep(2) 
		
		# check and send response
		while True:
			data = self.serial.read_serial()
			print(data)
			
			if data == 'WAITING-TYPES': # COMMAND MUST MATCH ARDUINO OUTPUT
				reply = self._setter(cmd)       # send sensor types
				print(reply)
			elif data  == "CALIBRATED": # COMMAND MUST MATCH ARDUINO OUTPUT
				self.flush()
				break
	
	
	def setup(self, type0:str=None, type1:str=None, type2:str=None, type3:str=None):
		'''Assigns the sensor types for each port and performs an initial calibration
		of the temperature sensors. Defaults to pre-programmed sensor types if no
		parameters are provided. 
		
		Parameters
		----------
		type0 : char
			Sensor type (N, K, J, etc) of channel 0
		
		type1 : char
			Same as before but for channel 1
		
		type2 : char
			Same as before but for channel 2
		
		type3 : char
			Same as before but for channel 3
		'''
		# Custom types
		if type0 and type1 and type2 and type3:
			cmd = str(type0) + str(type1) + str(type2) + str(type3)
			self._setup(cmd)
		# Default types
		else:
			self._setup('0')
		
		# store types
		self._set_sensor_type_json()
	
	
	# ================ wrappers ================
	def close(self):
		''' Close the serial connection. Must call at the end of program execution to ensure \
		    the connection does not remain active.
		'''
		self.serial.close()
	
	
	def flush(self):
		''' Block program execution and wait for serial commands to be written
		'''
		self.serial.flush()
	
	
	
	def is_active(self):
		'''Returns whether the serial connection is active (True) or disabled (False)
		
		Returns
		-------
		is_active: bool
		'''
		return self.serial.is_active()
	
	def send_serial_command(self, cmd):
		''' Send an arbitrary serial command and receive a reply from the Arduino
		
		Parameters
		----------
		cmd: str
			String that contains serial command
		
		Returns
		-------
		reply : str
			String that contains serial reply from the Arduino
		'''
		return self._setter(cmd)
	
	
	# ================ functions ================
		# private
	def _getter(self, cmd, out=None):
		''' Private function to send a specific serial command and receive a parsed reply
		
		Parameters
		----------
		cmd: str
			String that contains serial command

		out: str
			Optional parameter. Specifies the type of output after parsing the string.
			Can be: None, "param", "str", "str_arr"
		
		Returns
		-------
		None : Dict
			If no parameter is specified, the function returns a dictionary with \
			all data from a parsed serial command
		
		param : list[float]
			returns list[float] of the captured coefficients received from the arduino
			
		str : str
			returns the raw string received via serial
		
		str_arr : list[str]
			returns the received serial command split by a comma (,) delimiter 
		'''
		self.serial.write_serial(cmd)
		return self.serial.read_data(out)
	
	
	def _setter(self, cmd):
		''' Private function to send an arbitrary serial command a receive the raw reply. \
		    See: send_serial_command()
		'''
		self.serial.write_serial(cmd)
		return self.serial.read_serial()	# recognize message is received
	
	
		# sensor
	def get_filter(self):
		'''Gets the temperature measurements that have been smoothed by a kalman filter
		
		Returns
		-------
		temp: list[float]
			Temperature of each port: [port1, port2, port3, port4]
		'''
		cmd = "0"
		return self._getter(cmd, out='param')
	
	
	def get_raw(self):
		'''Gets the raw temperature measurements of the sensor(s)
		
		Returns
		-------
		temp: list[float]
			Temperature of each port: [port1, port2, port3, port4]
		'''
		cmd = "1"
		return self._getter(cmd, out='param')
	
	
	def get_target(self):
		'''Gets the target temperatures of the PID controllers
		
		Returns
		-------
		temp: list[float]
			Target temperature of each port: [port1, port2, port3, port4]
		'''
		cmd = "2"
		return self._getter(cmd, out='param')
	
	
		# target
	def set_target(self, ch, target): 
		'''Sets the target temperature of the PID controller of a specific port
		
		Parameters
		----------
		ch : int
			Channel of the temperature shield. Options are: {0, 1, 2, 3}
		
		target : float
			Target temperature (in Celsius) for the PID controller of specified channel
		'''
		cmd = "3," + str(ch) + "," + str(target) 
		self._setter(cmd) 
	
	def set_target_all(self, targ0, targ1, targ2, targ3): 
		'''Sets the target temperature for all PID controllers simultaneously
		
		Parameters
		----------
		targ0 : float
			Target temperature (in Celsius) for the PID controller of channel 0
		
		targ1 : float
			Target temperature (in Celsius) for the PID controller of channel 1
		
		targ2 : float
			Target temperature (in Celsius) for the PID controller of channel 2
		
		targ3 : float
			Target temperature (in Celsius) for the PID controller of channel 3
		'''
		cmd = "3,4," + str(targ0) + "," + str(targ1) + "," + str(targ2) + "," + str(targ3)
		self._setter(cmd)
	
	
		# PID controller
	def get_pid(self, ch):
		'''Gets the PID coefficients of a specific channel
		Parameters
		----------
		ch : int
			Channel of the Temperature shield. Options are: {0, 1, 2, 3}
		
		Returns
		-------
		coefficients: list[float]
			values in the order: [Proportional, Integral, Derivative]
		'''
		cmd = "4," + str(ch)
		return self._getter(cmd, out='param')
	
	
	def set_pid(self, ch, kp, ki, kd):
		'''Sets the PID coefficients of a specific channel
		
		Parameters
		----------
		ch : int
			Channel of the Temperature shield. Options are: {0, 1, 2, 3}
		
		kp : float
			Proportional coefficient
		
		ki : float
			Integral coefficient
		
		kd : float
			Derivative coefficient
		'''
		cmd = "5," + str(ch) + "," + str(kp) + "," + str(ki) + "," + str(kd)
		self._setter(cmd)
		self._set_pid_json(ch, kp, ki, kd)
	
	
			# input limits
	def get_in_limit(self, ch):
		"""Gets the target temperature limits (in Celsius) for the PID controller 
		of a specific channel
		
		Parameters
		----------
		ch : int
			Channel of the Temperature shield. Options are: {0, 1, 2, 3}
		
		Returns
		-------
		limits: list[float]
			values in the order: [input_max, input_min]
		"""
		cmd = "6," + str(ch)
		return self._getter(cmd, out='param')
	
	
	def set_in_limit(self, ch, imax, imin):
		'''Sets the target temperature limits (in Celsius) for the PID controller \
		of a specific channel
		
		Parameters
		----------
		ch : int
			Channel of the Temperature shield. Options are: {0, 1, 2, 3}
		
		imax : float
			Maximum allowable target temperature (in Celsius)
		
		imin : float
			Minimum allowable target temperature (in Celsius)
		'''
		cmd = "7," + str(ch) + "," + str(imax) + "," + str(imin)
		self._setter(cmd)
		self._set_in_limit_json(ch, imax, imin)
	
	
		# Filters
			# alpha-beta
	def get_ab_filter(self, ch):
		'''Gets the coefficients of the alpha-beta filter assigned to the PID controller
		of a specific channel
		
		Parameters
		----------
		ch : int
			Channel of the Temperature shield. Options are: {0, 1, 2, 3}
		
		Returns
		-------
		coefficients: list[float]
			 values in the order: [alpha, beta]
		'''
		cmd = "8," + str(ch)
		return self._getter(cmd, out='param')
	
	
	def set_ab_filter(self, ch, alpha, beta):
		'''Sets the coefficients of the alpha-beta filter assigned to the PID controller \
		of a specific channel
		
		Parameters
		----------
		ch : int
			Channel of the Temperature shield. Options are: {0, 1, 2, 3}
		
		alpha: float
			Alpha coefficient (signal smoothing): 0 < alpha < 1
		
		beta : float
			Beta coefficient (derivative smoothing): 0 < beta < 1
		'''
		cmd = "9," + str(ch) + "," + str(alpha) + "," + str(beta)
		self._setter(cmd)
		self._set_ab_filter_json(ch, alpha, beta)
	
	
			# kalman
	def get_k_filter(self, ch):
		'''Gets the coefficients of the kalman filter assigned to the temperature \
		measurements of a specific channel
		
		Parameters
		----------
		ch : int
			Channel of the Temperature shield. Options are: {0, 1, 2, 3}
		
		Returns
		-------
		Coefficients: list[float]
			values in the order: [error, noise]
		'''
		cmd = "10," + str(ch)
		return self._getter(cmd, out='param')
	
	
	def set_k_filter(self, ch, error, noise):
		'''Sets the coefficients of the kalman filter assigned to the temperature \
		measurements of a specific channel
		
		Parameters
		----------
		ch : int
			Channel of the Temperature shield. Options are: {0, 1, 2, 3}
		
		error : float
			Standard deviation of the measurement. Must be positive.
		
		noise : float
			Process noise parameter. A smaller value smooths the data at the cost of lag: \
			0 < noise < 1
		'''
		cmd = "11," + str(ch) + "," + str(error) + "," + str(noise)
		self._setter(cmd)
		self._set_k_filter_json(ch, error, noise)
	
	
	def set_k_filter_state(self, ch, value):
		'''Sets state of the kalman filter assigned to specific channel
		
		Parameters
		----------
		ch : int
			Channel of the Temperature shield. Options are: {0, 1, 2, 3}
		
		value : float
			Value of the filter (in Celsius)
		'''
		cmd = "12," + str(ch) + "," + str(value)
		self._setter(cmd)
	
	
		# Sensors
	def get_sensor_type(self): 	# NOTE: returns characters
		'''Retrieves the sensor type (N, K, J, etc) that was assigned \
		to each port of the CN0391 
		
		Returns
		-------
		types: list[char]
			Sensor type of each port: [port1, port2, port3, port4]
		'''
		cmd = "13"
		return self._getter(cmd, out='str_arr')[1:]
	
	
		#Enable / Disable controllers
	def set_enable(self, ch):
		'''Enables the PID controller of a specific channel
		
		Parameters
		----------
		ch : int
			Channel of the Temperature shield. Options are: {0, 1, 2, 3}
		'''
		cmd = "14," + str(ch)
		self._setter(cmd)
		self._set_enable_json(ch)
	
	
	def set_enable_all(self): 
		''' Enables the PID controller of every channel
		'''
		cmd = "14,4"
		self._setter(cmd)
		self._set_enable_all_json()
	
	
	def set_disable(self, ch):
		'''Disables the PID controller of a specific channel
		
		Parameters
		----------
		ch : int
			Channel of the Temperature shield. Options are: {0, 1, 2, 3}
		'''
		cmd = "15," + str(ch)
		self._setter(cmd)
		self._set_disable_json(ch)
	
	
	def set_disable_all(self): 
		''' Disables the PID controller of every channel
		'''
		cmd = "15,4"
		self._setter(cmd)
		self._set_disable_all_json()
	
	
	def get_enable(self):
		'''Retrieves which ports have a PID controller actively monitoring their temperature.
		A port can be active (True) or disabled (False)
		
		Returns
		-------
		state: list[bool]
			condition of each port: [port1, port2, port3, port4]
		'''
		cmd = "16"
		data = self._getter(cmd, out='param')#.tolist() -> part of arduinoserial
		return [elem == True for elem in data]	# convert to boolean
	
	
		# timers
	def get_timer(self):
		'''Retrieves the time (in seconds) each port has been actively controlling its temperature.
		
		Returns
		-------
		times: list[float]
			Time each port has been active: [port1, port2, port3, port4]
		'''
		cmd = "17"
		return self._getter(cmd, out='param')
	
	
	def get_timeout(self):
		'''Retrieves the time (in seconds) each port is allowed to control its temperature.
		
		Returns
		-------
		timeouts: list[float]
			Time each port can be active: [port1, port2, port3, port4]
		'''
		cmd = "18"
		return self._getter(cmd, out='param')
	
	
	def set_timeout(self, ch, time):
		'''Sets the time a PID controller will be enabled for a specific channel
		
		Parameters
		----------
		ch : int
			Channel of the Temperature shield. Options are: {0, 1, 2, 3}
		
		time : float
			Time (in seconds) the controller will be active
		'''
		cmd = "19," + str(ch) + "," + str(time)
		self._setter(cmd)
		self._set_timeout_json(ch, time)
	
	
	def set_timeout_inf(self, ch):
		'''Allow the PID controller at a specific port to run forever
		
		Parameters
		----------
		ch : int
			Channel of the Temperature shield. Options are: {0, 1, 2, 3}
		'''
		cmd = "19," + str(ch) + ",-1"	# send value for infinite time. See `Commands.h`
		self._setter(cmd)
		self._set_timeout_inf_json(ch)
	
	
	# ================ JSON coefficients ================
	
	#----- Reading -------
	
	def load_json_file(self, path):
		'''Load the JSON file containing the controller parameters
		
		Parameters
		----------
		path : str
			Path to the JSON file
		'''
		with open(path) as outfile:
			self.json_data = json.load(outfile)
	
	
	def set_json_coefficients(self):
		'''Load the coefficients contained in the JSON file. Requires Calling `load_json_file` \
		beforehand. Will not work otherwise. 
		'''
		# retrieve data
		parameters = self.json_data["parameters"]
		
		for channel, param in enumerate(parameters):
			# load paramters
			kp    = param["kp"]
			ki    = param["ki"]
			kd    = param["kd"]
			alpha = param["alpha"]
			beta  = param["beta"]
			error = param["error"]
			noise = param["noise"]
			imax  = param["imax"]
			imin  = param["imin"]
			timeout = param["timeout"]
			enable  = param["enable"]
			
			# write state
			self.set_pid(channel, kp, ki, kd)
			self.set_ab_filter(channel, alpha, beta)
			self.set_k_filter(channel, error, noise)
			self.set_in_limit(channel, imax, imin)
			self.set_timeout(channel, timeout)
			
			if enable:
				self.set_enable(channel)
	
	
	def get_json_data(self):
		'''Retrieves the parameters and coefficients required to configure the serial communication \
		and PID controllers.
		
		Returns
		-------
		data : Dict
			Dictionary with all of the parameters and coefficients parsed from the JSON File.
		'''
		return self.json_data
	
	#----- Writing -------
	
	def _construct_json(self) -> dict:
		'''Constructs a dictionary with the form needed to store the controller parameters.
		
		Returns
		-------
		data : Dict
			dictionary with every value set to `None`
		'''
		coeffs = {
			"kp":    None,
			"ki":    None,
			"kd":    None,
			"alpha": None,
			"beta":  None,
			"error": None,
			"noise": None,
			"imax":  None,
			"imin":  None,
			"timeout": None,
			"enable":  None
		}
		data = {
			"baud_rate":    None,
			"serial_port":  None,
			"sensor_types": [None, None, None, None],
			"parameters": [ coeffs.copy(), \
			                coeffs.copy(), \
			                coeffs.copy(), \
			                coeffs.copy() ] # ensure a hard copy [:]
		}
		
		return data
	
	
	def save_json_file(self, path):
		'''Save a JSON file with the current controller parameters
		
		Parameters
		----------
		path : str
			Path to the JSON file
		'''
		with open(path, "w") as outfile:
			json_object = json.dumps(self.json_data, indent=4)
			outfile.write(json_object)	# store file
	
	#-- functions to store state in JSON file:
	
	def get_device_coefficients(self):
		'''Gets the coefficients currently loaded on the Arduino and prepares them to be stored
		in a JSON file.
		'''
		# get enabled ports
		is_enabled = self.get_enable()
		timeout = self.get_timeout()
		
		# set json data
		for ch in range(0, 4):
			# enable
			if is_enabled[ch]:
				self._set_enable_json(ch)
			else:
				self._set_disable_json(ch)
			
			# set state
				# pid
			var = self.get_pid(ch)
			self._set_pid_json( ch, var[0], var[1], var[2] )
			
				# alpha-beta filter
			var = self.get_ab_filter(ch)
			self._set_ab_filter_json( ch, var[0], var[1] )
			
				# kalman filter
			var = self.get_k_filter(ch)
			self._set_k_filter_json( ch, var[0], var[1] )
			
				# input limits
			var = self.get_in_limit(ch)
			self._set_in_limit_json( ch, var[0], var[1] )
			
				# timeout
			self._set_timeout_json( ch, timeout[ch] )
	
	
	def _set_pid_json(self, ch, kp, ki, kd):
		'''Stores the PID coefficients in a dictionary
		
		Parameters
		----------
		See `set_pid` for details
		'''
		self.json_data["parameters"][ch]["kp"] = kp
		self.json_data["parameters"][ch]["ki"] = ki
		self.json_data["parameters"][ch]["kd"] = kd
	
	
	def _set_in_limit_json(self, ch, imax, imin):
		'''Stores the input limits in a dictionary
		
		Parameters
		----------
		See `set_in_limit` for details
		'''
		self.json_data["parameters"][ch]["imax"] = imax
		self.json_data["parameters"][ch]["imin"] = imin
	
	
	def _set_k_filter_json(self, ch, error, noise):
		'''Stores the coefficients for a kalman filter in a dictionary
		
		Parameters
		----------
		See `set_k_filter` for details
		'''
		self.json_data["parameters"][ch]["error"] = error
		self.json_data["parameters"][ch]["noise"] = noise
	
	
	def _set_ab_filter_json(self, ch, alpha, beta):
		'''Stores the coefficients for an alpha-beta filter in a dictionary
		
		Parameters
		----------
		See `set_ab_filter` for details
		'''
		self.json_data["parameters"][ch]["alpha"] = alpha
		self.json_data["parameters"][ch]["beta"] = beta
	
	
	def _set_enable_json(self, ch):
		'''Stores whether a PID controller is enabled in a dictionary
		
		Parameters
		----------
		See `set_enable` for details
		'''
		self.json_data["parameters"][ch]["enable"] = 1
	
	
	def _set_enable_all_json(self):
		'''Stores whether all PID controllers are enabled in a dictionary
		
		Parameters
		----------
		See `set_enable_all` for details
		'''
		for ch in range(0,4):
			self._set_enable_json(ch)
	
	
	def _set_disable_json(self, ch):
		'''Stores whether a PID controller is disabled in a dictionary
		
		Parameters
		----------
		See `set_disable` for details
		'''
		self.json_data["parameters"][ch]["enable"] = 0
	
	
	def _set_disable_all_json(self):
		'''Stores whether all PID controllers are disabled in a dictionary
		
		Parameters
		----------
		See `set_disable_all` for details
		'''
		for ch in range(0,4):
			self._set_disable_json(ch)
	
	
	def _set_timeout_json(self, ch, time):
		'''Stores the time a PID controller can be enabled in a dictionary
		
		Parameters
		----------
		See `set_timeout` for details
		'''
		self.json_data["parameters"][ch]["timeout"] = time
	
	
	def _set_timeout_inf_json(self, ch):
		'''Stores whether a PID controller stay on indefinitely in a dictionary
		
		Parameters
		----------
		See `set_timeout_inf` for details
		'''
		self.json_data["parameters"][ch]["timeout"] = -1 
		# Uses default for infinite time.
		# See: /arduino/Serialcom.ino
	
	
	def _set_sensor_type_json(self):
		'''Stores the sensor type for each for in a dictionary
		
		Parameters
		----------
		See `set_sensor_type` for details
		'''
		self.json_data["sensor_types"] = self.get_sensor_type()


