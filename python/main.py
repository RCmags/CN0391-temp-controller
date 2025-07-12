# Global libraries
import matplotlib.pyplot as plt
import numpy as np
import time
import json

# local file
import KeyboardThread as kb
import TempControllerCN0391 as cntl

#=========================================================================================

def load_parameters(path):
	with open(path) as file:
		data = json.load(file)
	return data

def load_coefficients(data, controller):
	# retrieve data
	parameters  = data["parameters"]
	
	for channel, param in enumerate(parameters):
		# load paramters
		kp    = param["kp"]
		ki    = param["ki"]
		kd    = param["kd"]
		alpha = param["alpha"]
		beta  = param["beta"]
		error = param["error"]
		noise = param["noise"]
		imax  = param["imax"]
		imin  = param["imin"]
		
		# write state
		controller.set_pid(channel, kp, ki, kd)
		controller.set_ab_filter(channel, alpha, beta)
		controller.set_k_filter(channel, error, noise)
		controller.set_in_limit(channel, imax, imin)
		
		"""
		# check
		print( "ch:"+ str(channel) )
		print( controller.get_pid(channel) )
		print( controller.get_ab_filter(channel) )
		print( controller.get_k_filter(channel) )
		print( controller.get_in_limit(channel) )
		"""

#-----------------------------------------------------------------------------------------

def main(data, window, ylims, nsamples=10):
	# ---- Serial ----
	controller = cntl.TempControllerCN0391( data["serial_port"], data["baud_rate"] )
	
		# setup
	controller.setup()		# Note: must come before setting coefficients 
	controller.set_enable(ch=1)
	load_coefficients(data, controller)
	
	# ---- keyboard inputs ----
	def callback(string):
		if string == "exit":
			keythread.stop()
			controller.set_disable_all()	# turn off heater
			controller.flush()
			controller.close()
		else:
			controller.serial.write_serial(string) 
			controller.flush()

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
	
	print("CLOSED-PYTHON")
	
#=========================================================================================

if __name__ == '__main__':
	config = load_parameters('coefficients.json')
	main(data=config, window=120, ylims=[20, 80])

"""
Note: the Serial port changes with operating system:
		On Linux and macOS, use '/dev/ttyACM0' or similar
		On Windows, use 'COM3' or similar

PORT        String of the Serial port used by the arduino
BAUD_RATE   Baud rate used by the arduino. Must match the value in "Constants.h"
X_WINDOW    Horizontal size of x-axis of the real time plot
Y_LIMS      Range of visible temperatures in the real time plot; [minimum, maximum]
"""

