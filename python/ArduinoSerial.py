import serial
import numpy as np

# NOTE: keyboard inputs are separate from receiving and sending serial commands.

#--- CONSTANTS ---
_END_CHAR = '\n' # see: arduino/Constants.h
_TIMEOUT  = 2    # seconds

#=========================================================================================

class SerialCommunication:
	
	def __init__(self, port, baud_rate):
		self.arduino = serial.Serial(port, baud_rate, timeout=_TIMEOUT) 
		self._isActive = True
	
	# helpers
	def close(self):
		self.arduino.close()
		self._isActive = False
	
	def is_active(self):
		return self._isActive
	
	def flush(self):
		self.arduino.flushInput()
	
	def _strip_string(self, string):
		string = string.replace('\r', '')
		string = string.replace('\n', '')
		return string
	
	# read serial strings
	def read_serial(self): # add to separate thread
		try: 
			return self.arduino.readline().decode('utf-8')	# blocks | must enable timeout
		except:
			return ""
	
	def _parse_serial_string(self, string, out_type=None):
		# strip characters
		string = self._strip_string(string)
		
		# separate inputs
		string_arr = string.split(",") 		# delimiter. Must match DELIM_CHAR from Arduino.
		
		if out_type == "str":
			return string_arr
		
			# outputs
		output = np.array( [] )
		parseIsGood = True
		
		# convert parameters
		function = string_arr[0]
		
		if out_type == "func":
			return function
		
		# parse values to float
		if len(string_arr) > 1:
			parameters = string_arr[1:]
			
			for element in parameters:
				try:
					number = float(element)
				except:
					number = 0 							# default value
					parseIsGood = False
					break;
				
				output = np.append(output, number)
				output = output.reshape(-1,1) 			# array flip to vertical
		
		if out_type == "param" or out_type == None:
			return output
		
		if out_type == "all":
			return {'param': output, 'func': function, 'flag': parseIsGood, 'str': string} 
	
	
	def read_data(self, out_type=None):
		string = self.read_serial()
		return self._parse_serial_string(string, out_type)
	
	# write serial strings
	def write_serial(self, string):
		string = f'{string}{_END_CHAR}' # add end-of-line to avoid blocking. ESSENTIAL
		cmd = string.encode('utf-8')
		self.arduino.write(cmd)


