import serial
import numpy as np
import time
# NOTE: keyboard inputs are separate from receiving and sending serial commands.

#--- CONSTANTS ---
_END_CHAR = '\n' # see: arduino/Constants.h
_TIMEOUT  = 5    # seconds | Maximum time to 
_DELAY    = 0.25 # seconds | Delay that allows serial command to be sent

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
	
	def flush(self): # adjust?
		self.arduino.reset_input_buffer()
		self.arduino.reset_output_buffer()
	
	def _strip_string(self, string):
		string = string.replace('\r', '')
		string = string.replace('\n', '')
		return string
	
	# read serial strings
	def read_serial(self): 	# add to separate thread
		data = self.arduino.readline().decode('utf-8') # blocks unless timeout is given
		return data
	
	# write serial strings
	def write_serial(self, string):
		string = f'{string}{_END_CHAR}'     # add end-of-line to avoid blocking. ESSENTIAL
		cmd = string.encode('utf-8')
		self.arduino.reset_output_buffer()  # ensure no unwanted data is sent
		self.arduino.write(cmd)
		time.sleep(_DELAY)					# wait for message to be received
	
	# parse command
	def _parse_serial_string(self, string):
		# strip characters
		string = self._strip_string(string)
		
		# separate inputs
		string_arr = string.split(",") 		# delimiter. Must match DELIM_CHAR from Arduino.

			# outputs
		output = np.array( [] )
		parseIsGood = True
		
		# convert parameters
		function = string_arr[0]
		
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
		
		return {'param': output, 'func': function, 'flag': parseIsGood, \
		          'str': string, 'str_arr': string_arr} 
	
	def read_data(self):
		string = self.read_serial()
		return self._parse_serial_string(string)
	



