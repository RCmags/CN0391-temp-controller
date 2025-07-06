import serial
import numpy as np
# NOTE: keyboard inputs are separate from receiving and sending serial commands.

#=========================================================================================

class SerialCommunication:
	
	def __init__(self, port, baud_rate):
		self.arduino = serial.Serial(port, baud_rate)
	
	# helpers
	def close(self):
		self.arduino.close()
	
	def flush(self):
		self.arduino.flushInput()
	
	def _strip_string(self, string):
		string = string.replace('\r', '')
		string = string.replace('\n', '')
		return string
	
	# read serial strings
	def read_serial_string(self): # add to separate thread
		try: 
			data = self.arduino.readline().decode('utf-8') # blocking function -> add thread?
			return self._strip_string(data)
		except:
			return ""
	
	def parse_serial_string(self, string):
		# strip characters
		#string = self._strip_string(string)
		
		# separate inputs
		string_arr = string.split(",") 		# delimiter. Must match DELIM_CHAR from Arduino.
		
			# outputs
		output = np.array( [] )
		parseIsGood = True
		
		# convert parameters
		function = string_arr[0]
		
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
		
		return {'param': output, 'func': function, 'flag': parseIsGood, 'str': string} # provide reduced output
	
	def read_data(self):
		string = self.read_serial_string()
		return self.parse_serial_string(string)
	
	# write serial strings
	def write_data(self, string):
		data = string.encode('utf-8')
		self.arduino.write(data)


