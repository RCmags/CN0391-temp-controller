# Global libraries
import matplotlib.pyplot as plt
import numpy as np
import time

# local file
import ArduinoSerial
import KeyboardThread as kb
import TempControllerCN0391 as tctl
#import Commands

#=========================================================================================

def main(port, baud_rate, window, ylims, nsamples=10):
	# ---- State ----
	# Serial
	controller = tctl.TempControllerCN0391(port, baud_rate)
	
	# keyboard inputs
	def callback(string):
		controller.serial.write_data(string)
		if string == "exit":
			keythread.stop()
			controller.close()
	
	keythread = kb.KeyboardThread( function=callback )
	keythread.start() # must start thread
	
	#--- setup ---
	print("pre-setup")
	
	controller.setup('T', 'J', 'K', 'N')
	#controller.setup()
	
	print("past-setup")
	
	#--- test functions ---
	
	print( controller.get_filter() )
	print( controller.get_raw() )
	
	controller.set_target(1, 45)
	print( controller.get_target() )
	
	controller.set_target_all(10, 20, 30, 40)
	print( controller.get_target() )
	
	controller.set_pid(0, 4, 2, 3)
	print( controller.get_pid(0) )
	
	controller.set_in_limit(0, 50, 25)
	print( controller.get_in_limit(0) )
	
	controller.set_ab_filter(0, 0.1, 0.2)
	print( controller.get_ab_filter(0) )
	
	controller.set_k_filter(0, 0.3, 0.4)
	print( controller.get_k_filter(0) )
	
	controller.set_k_filter_state(0, 100)
	print( controller.get_filter() )
	
	print( controller.get_sensor_type() )
	
	controller.set_enable(0)
	print( controller.get_enable() )
	
	controller.set_enable_all()
	print( controller.get_enable() )
	
	controller.set_disable(0)
	print( controller.get_enable() )
	
	controller.set_disable_all()
	print( controller.get_enable() )
	
	print( controller.get_timer() )
	
	controller.set_timeout(0, 100)
	print( controller.get_timeout() )
	
	controller.set_timeout_inf(0)
	print( controller.get_timeout() )
	
	
	#---- loop ----
	print("enter-loop")
	while keythread.is_active():
		data = controller.serial.read_data("all") # capture all data
		
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
2. [x] add parameter to select type out of output of read_data()
3. [x] add default baud rate to TempController constructor
4. [x] add setup(types) function to TempController
"""

"""
Pending | Main program:
1. get real-time data plot to work
2. load coefficients from json file. Load into controller.
"""
