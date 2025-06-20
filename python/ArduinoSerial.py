import serial
import threading
import numpy as np

#=========================================================================================

# Class to get keyboard inputs via non-blocking function
# See: https://stackoverflow.com/questions/2408560/non-blocking-console-input

class KeyboardThread(threading.Thread):

    def __init__(self, input_cbk = None, name='keyboard-input-thread'):
        self.input_cbk = input_cbk
        super(KeyboardThread, self).__init__(name=name, daemon=True)
        self.start()
	
    def run(self):
        while True:
            self.input_cbk(input()) #waits to get input + Return

#=========================================================================================

class SerialCommunication:
	
	def __init__(self, port, baud_rate):
		self.loop_flag = True
		self.arduino = serial.Serial(port, baud_rate)
		self.keythread = KeyboardThread(self.__inputCallback)
	
	# loop flag / exit command
	def getFlag(self):
		return self.loop_flag
	
	def resetFlag(self):
		self.loop_flag = True
	
	# close serial connection
	def close(self):
		self.arduino.close()
	
	# call back function to get keyboard input
	def __inputCallback(self, input_data): 
		if str(input_data) == "exit":		# Special input to exit loop 
			self.loop_flag = False
		else:
			encoded_data = input_data.encode('utf-8')
			self.arduino.write(encoded_data)
	
	# read serial input
	def readSerialString(self):
		try: 
			return self.arduino.readline().decode('utf-8') 
		except:
			return ""
	
	# parse string of the form [number1, number2, number3...]
	def readData(self):
		string = self.readSerialString();
		string_arr = string.split(",") 		# delimiter
		
		output = np.array( [] )
		
		for element in string_arr:
			try:
				number = float(element)
			except:
				number = 0 					# default value
		
			output = np.append(output, number)
		
		return output.reshape(-1,1) 		# array flip to vertical
	
	# get average of multiple measurements
	def averageSignal(self, n_sum):
		self.arduino.flushInput()
		
		summation = 0
		for i in range(n_sum):
			summation = summation + self.readData()

		return summation / n_sum
	
