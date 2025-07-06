# Global libraries
import matplotlib.pyplot as plt
import numpy as np
import time

# local file
import ArduinoSerial
import KeyboardThread as kb

#=========================================================================================

def main(port, baud_rate, window, ylims, nsamples=10):
	# ---- State ----
	# Serial 
	serialCom = ArduinoSerial.SerialCommunication(port, baud_rate)
	time.sleep(2)
	
	
	def callback(string):
		print("INPUT, " + string)
		serialCom.write_serial_string(string)
		
		if string == "exit":
			keythread.stop()
			serialCom.close()
	
	
	keythread = kb.KeyboardThread(callback)
	keythread.start() # must start thread
	
	
	#---- loop ----
	
	while True:
		try:
			data, function, parseIsGood = serialCom.read_data() # blocking function. Waits for input to arrive 
			print(function, data, parseIsGood)
		except:
			continue
		
		#print("END-LOOP")

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

