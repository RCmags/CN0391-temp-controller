# Global libraries
import matplotlib.pyplot as plt
import numpy as np
import time

# local file
import ArduinoSerial
import KeyboardThread as kb
import Commands

#=========================================================================================

def main(port, baud_rate, window, ylims, nsamples=10):
	# ---- State ----
	# Serial 
	serialCom = ArduinoSerial.SerialCommunication(port, baud_rate)
	time.sleep(2)
	
	# keyboard inputs
	def callback(string):
		serialCom.write_data(string)

		if string == "exit":
			keythread.stop()
			serialCom.close()
	
	keythread = kb.KeyboardThread( function=callback )
	keythread.start() # must start thread
	
	#--- setup ---
	serialCom.write_data( Commands.set_sensor_type('N', 'N', 'N', 'N') )
	
	#---- loop ----
	while keythread.is_active():
		#data, function, parseIsGood, string = serialCom.read_data() # blocking function
		data = serialCom.read_data()
		
		if data["flag"]:
			print( data["str"] )
		

#=========================================================================================

if __name__ == '__main__':
	# Inputs
	PORT 		= "/dev/ttyACM0"	# String of the Serial port used by the arduino
	BAUD_RATE 	= 9600				# Baud rate used by the arduino. Must match the value in "Constants.h"
	X_WINDOW 	= 120				# Horizontal size of x-axis of the real time plot
	Y_LIMS 		= [20, 80]			# Range of visible temperatures in the real time plot; [minimum, maximum]
	
	main( PORT, BAUD_RATE, X_WINDOW, Y_LIMS )
	print("CLOSED-PYTHON")

# Note: the Serial port changes with operating system:
# 		On Linux and macOS, use '/dev/ttyACM0' or similar
#		On Windows, use 'COM3' or similar


"""
# get average of multiple measurements
def averageSignal(self, n_sum):
	self.arduino.flushInput()
	
	summation = 0
	for i in range(n_sum):
		summation = summation + self.readData()

	return summation / n_sum
"""

"""
Pending | Controller class:
1. [x] embed Commands.py directly into TempController
2. add parameter to select type out of output of read_data()
3. [x] add default baud rate to TempController constructor
4. add setup(types) function to TempController
"""

"""
Pending | Main program:
1. get real-time data plot to work
2. load coefficients from json file. Load into controller.
"""
