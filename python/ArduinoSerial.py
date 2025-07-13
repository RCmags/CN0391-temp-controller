import serial
import numpy as np
import time
# NOTE: keyboard inputs are separate from receiving and sending serial commands.

#--- CONSTANTS ---
_END_CHAR = '\n' # see: arduino/Constants.h
_TIMEOUT  = 20   # seconds | Time that serial reads are allowed to block code execution. 
_DELAY    = 1.0  # seconds | Required delay that allows serial command to be sent

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
		self.arduino.flush()
	
	def reset(self): 
		self.arduino.reset_input_buffer()
		self.arduino.reset_output_buffer()
	
	def _strip_string(self, string):
		string = string.replace('\r', '')
		string = string.replace(_END_CHAR, '')
		return string
	
	# read serial strings
	def read_serial(self): 	# add to separate thread
		data = self.arduino.readline().decode('utf-8') # blocks unless timeout is given
		self.arduino.flush()				# clear buffers
		data = self._strip_string(data)
		return data
	
	# write serial strings
	def write_serial(self, string):
		string = f'{string}{_END_CHAR}'     # add end-of-line to avoid blocking. ESSENTIAL
		cmd = string.encode('utf-8')
		self.arduino.reset_output_buffer()  # ensure no unwanted data is sent
		self.arduino.write(cmd)
		self.arduino.flush()				# ensure data is written
		time.sleep(_DELAY)					# wait for message to be received
	
	# parse command
	def _parse_serial_string(self, string, out):
		if out == 'str':
			return string
		
		# separate inputs
		string_arr = string.split(",") 		# delimiter. Must match DELIM_CHAR from Arduino.

		if out == 'str_arr':
			return string_arr

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
		
		if out == 'param':
			return output
		
		# default to all output
		return {'param': output, 'func':    function,   'flag': parseIsGood, \
		          'str': string, 'str_arr': string_arr} 
	
	def read_data(self, out=None):
		string = self.read_serial()
		return self._parse_serial_string(string, out)
	



