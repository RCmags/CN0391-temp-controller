# Global libraries
import matplotlib.pyplot as plt
import numpy as np
import time
import json

# local file
import ArduinoSerial
import KeyboardThread as kb
import TempControllerCN0391 as cntl

#=========================================================================================

def load_coefficients(path, controller):
	with open(path) as file:
		data = json.load(file)
	# load parameters
	controller.set_pid( data["ch"], data["kp"], data["ki"], data["kd"] )

#------------

def main(port, baud_rate, window, ylims, nsamples=10):
	# ---- Serial ----
	controller = cntl.TempControllerCN0391(port, baud_rate)
	
		# setup
	controller.setup()
	controller.set_enable(ch=1) 
	load_coefficients('coefficients.json', controller)
	
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
Pending | Controller class:
5. add adjustable timeout for serial
"""

