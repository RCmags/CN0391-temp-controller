"""
Self contained module to easily communicate with an Arduino over serial. It is intended to be used
with commands that have the form of a comma delimited string:

```
<command>,<channel>,<value1>,<value2>,... <valueN>
```

- The `<command>` can be a string or a number that indicates a specific function. 
- The `<channel>` is a number that indicates for which device the function is meant to apply to.
- The remaining `<values>` can be numbers or characters relevant to the aforementioned function.  
"""

# Global libraries
import serial
import time

# NOTE: keyboard inputs are separate from receiving and sending serial commands.

#--- CONSTANTS ---
_END_CHAR = '\n' # see: arduino/Constants.h
_TIMEOUT  = 60   # seconds | Time that serial reads are allowed to block code execution. 
_DELAY    = 0.5  # seconds | Required delay that allows serial command to be sent | 1 sec is safe but slow

# NOTE: Time delays need to be long enough to compensate for the slow execution of python

#===================================================================================================

class SerialCommunication:
	
	def __init__(self, port:str, baud_rate:int):
		'''Initialize serial communication with an Arduino.
		
		Parameters
		----------
		port: str
			Serial port used by the Arduino to communicate with the computer. \
			Required if `path` is not provided
		
		baud_rate: int
			Baud rate at which the Arduino is configured. Optional parameter that defaults to \
			hardcoded value if none is provided
		'''
		self.arduino = serial.Serial(port, baud_rate, timeout=_TIMEOUT) 
		self._isActive = True
	
	
	# helpers
	def close(self) -> None:
		'''Close the serial connection. It is necessary to run this command when a program 
		finished to ensure the serial connection does not stay active. 
		'''
		self.arduino.close()
		self._isActive = False
	
	
	def is_active(self) -> None:
		'''Returns whether the serial connection is active (True) or disabled (False)
		
		Returns
		-------
		is_active: bool
		'''
		return self._isActive
	
	
	def flush(self) -> None:
		''' Block program execution and wait for serial commands to be written
		'''
		self.arduino.flush()
	
	
	def reset(self) -> None: 
		''' Reset the input and output buffers of the serial connection. 
		'''
		self.arduino.reset_input_buffer()
		self.arduino.reset_output_buffer()
	
	
	def _strip_string(self, string:str) -> None:
		''' Private function remote trailing characters that are send via serial by the Arduino.
		
		Parameters
		----------
		string: str
			String that contains serial command
		
		Returns
		-------
		string : str
			String without end character and carriage return
		'''
		string = string.replace('\r', '')
		string = string.replace(_END_CHAR, '')
		return string
	
	
	# read serial strings
	def read_serial(self) -> str: 
		''' Read any incoming serial commands at the specified port. This is a blocking function
		that waits until data is received, or the connection times out. 
		
		Returns
		-------
		reply : str
			String that contains the captured data. Returns "" if no data is captured
			or if connection times out.
		'''
		data = self.arduino.readline().decode('utf-8') 	# blocks unless timeout is given
		self.arduino.flush()							# clear buffers
		data = self._strip_string(data)
		return data
	
	
	# write serial strings
	def write_serial(self, string:str) -> None:
		'''Write a string via serial to the specified port
		
		Parameters
		----------
		string: str
			String that is sent via serial
		'''
		string = f'{string}{_END_CHAR}'     # add end-of-line to avoid blocking. ESSENTIAL
		cmd = string.encode('utf-8')
		self.arduino.reset_output_buffer()  # ensure no unwanted data is sent
		self.arduino.write(cmd)
		self.arduino.flush()				# ensure data is written
		time.sleep(_DELAY)					# wait for message to be received
	
	
	# parse command
	def _parse_serial_string(self, string:str, out:str) -> str | list | dict:
		'''Private function that parses a serial string and converts it a list of coefficients
		
		Parameters
		----------
		string: str
			String that contains the comma separated values to be parsed
		
		out: str
			Optional parameter that determines the type of output. 
			For more details see: read_data()
		
		Returns
		-------
		data: Dict
			Dictionary with parsed data
		'''
		if out == 'str':
			return string
		
		# separate inputs
		string_arr = string.split(",") 		# delimiter. Must match DELIM_CHAR from Arduino.

		if out == 'str_arr':
			return string_arr

			# outputs
		output = []
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
					number = 0 	# default value
					parseIsGood = False
					break;
				output.append(number)
		
		if out == 'param':
			return output
		
		# default to all output
		return {'param': output, 'func':    function,   'flag': parseIsGood, \
		          'str': string, 'str_arr': string_arr} 
	
	
	def read_data(self, out:str=None) -> str | list | dict:
		'''Read any incoming serial commands and parse them to useful data
		
		Parameters
		----------
		out: str
			optional parameter that determines the desired output. 
			Can be: None, "param", "str", "str_arr"
		
		Returns
		-------
		None : Dict
			If no parameter is specified, the function returns a dictionary with \
			all data from a parsed serial command
		
		param : list[float]
			returns array of the captured coefficients received from the arduino
		
		str : str
			returns the raw string received via serial
		
		str_arr : list[str]
			returns the received serial command split by a comma (,) delimiter 
		'''
		string = self.read_serial()
		return self._parse_serial_string(string, out)



