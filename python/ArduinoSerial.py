import serial
import numpy as np
# NOTE: keyboard inputs are separate from receiving and sending serial commands.

#=========================================================================================

class SerialCommunication:
	
	def __init__(self, port, baud_rate):
		self.arduino = serial.Serial(port, baud_rate)
	
	# close serial connection
	def close(self):
		self.arduino.close()
	
	# read and write serial strings
	def read_serial_string(self): # add to separate thread
		try: 
			return self.arduino.readline().decode('utf-8') 
		except:
			return ""
	
	def write_serial_string(self, string):
		data = string.encode('utf-8')
		self.arduino.write(data)
	
	def flush(self):
		self.arduino.flushInput()
	
	# parse string 
	def read_data(self):
		string = self.read_serial_string()
		
		# strip characters
		string = string.replace('\r', '')
		string = string.replace('\n', '')
		
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
		
		return output, function, parseIsGood			# <string> <array> <bool>

