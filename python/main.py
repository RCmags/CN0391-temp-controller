# Global libraries
import matplotlib.pyplot as plt
import numpy as np
import time
#import json

# local file
import ArduinoSerial
import KeyboardThread as kb
import TempControllerCN0391 as cntl

#=========================================================================================

"""
def load_coefficients(path, function):
	with open(path) as file:
		data = file.read()	
	# load parameters
	function( data["ch"], data["kp"], data["ki"], data["kd"] )
"""

def main(port, baud_rate, window, ylims, nsamples=10):
	# ---- Serial ----
	controller = cntl.TempControllerCN0391(port, baud_rate)
	
	controller.setup()
		# port #1
	controller.set_enable(ch=1) 
	controller.set_pid(ch=1, kp=0.5, ki=0.01, kd=6)

	# ---- keyboard inputs ----
	def callback(string):
		if string == "exit":
			keythread.stop()
			controller.close()
		else:
			controller.serial.write_serial(string) 
			controller.serial.write_serial

	keythread = kb.KeyboardThread( function=callback )
	keythread.start() 		# must start thread
	
	# ---- Plots ----
	# Size of data
	time_init = 0
	time.sleep(2)			# wait for serial to reload
	
	y_init = sum( [controller.get_filter() for i in range(nsamples)] ) / nsamples
	n_data = np.size(y_init)
	
	# figure
	x = np.array( [] )
	y = np.array( [] )
	
	# plot graphs
	figure, ax = plt.subplots()
	lines = [ ax.plot(x, y, label=f'Port {i+1}')[0] for i in range(n_data) ] 

	#---- configure plot ----
	ax.legend()
	ax.set_xlim( [0, window] )
	ax.set_ylim( ylims )
	ax.set_xlabel("Seconds")
	ax.set_ylabel("Celcius")

	plt.ion()
	plt.show()
	
	#---- loop ----
	while controller.is_active(): 	# check connection
		try:
			# update plot data
			time_now = time.time()
			
			if np.size(x) == 0: 
				time_init = time_now
				y = y_init.copy()
			else:
				ynew = controller.get_filter()	# breaks when interrupt is called
				y = np.hstack( (y, ynew) )
			
			time_zero = time_now - time_init
			x = np.append(x, time_zero);
			
			if time_zero > window:
				y = y[:, 1:] 				# delete first vertical slice
				x = np.delete(x, 0)
				ax.set_xlim( [ x[0], x[-1] ] )
			
			# update plots
			for i in range(n_data):
				lines[i].set_xdata(x)
				lines[i].set_ydata( y[i] )
			
			figure.canvas.draw()
			figure.canvas.flush_events()
		
		except:
			continue
	
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
1. [x] get real-time data plot to work
2. load coefficients from json file. Load into controller.
"""
