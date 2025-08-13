# Global libraries
import matplotlib.pyplot as plt
import numpy as np
import time
import argparse

# local file
from temp_controller import KeyboardThread as kb
from temp_controller import TempControllerCN0391 as cntl

#===================================================================================================

def list_of_floats(string: str) -> list:
	'''Converts a string delimited by commas into an array of strings.
	
	Parameters
	----------
	string: str
		String of delimited parameters
	
	Returns
	-------
	data : list(str)
		array of strings
	'''
	return list( map(float, string.split(',')) )


def positive_int(string: str) -> int:
	'''Converts a string to positive integer. 
	
	Parameters
	----------
	string: str
		String that contains numbers
	
	Returns
	-------
	num : int
		Integer that is at least 1. Can be greater than one.
	'''
	num = int(string)
	if num > 0:
		return num
	else:
		return 1


def captureInputs() -> object: 
	'''Captures command line parameters provided by the user when calling the script. 
	For example:
	
	```bash
	python3 main.py --path "./config.json" --ylims 30, 100 --window 100
	```
	
	At the very minimum, the serial port must be specified:
	
	```bash
	python3 main.py --port "/dev/ttyACM0"
	```
	
	Parameters
	----------
	port : str
		Path to the Serial Port that the Arduino is connected to. Must be provided if `path` is not.
	
	path : str
		Path to the JSON configuration file for the controller. Must be provided if `port` is not.
		This parameter will override any value for `port`.
	
	ylims: list(float)
		Optional parameter to set the maximum and minimum values of Y axis.
	
	window : float
		Optional parameter to set the width of the X axis time window. 
	
	nsamples : int
		Optional parameter for the number of data samples used to initially configure the plots. \
		Must be larger than zero. 
	
	Returns
	-------
	data : object
		Object with the data mentioned in the description. It contains the fields: \
		`path`, `ylims`, and `window`.
	'''
	parser = argparse.ArgumentParser()
	
	# port configuration
	group = parser.add_mutually_exclusive_group(required=True)
	
	group.add_argument("--port", type=str, default=None, \
	                   help="Path to the Serial Port the Arduino is connected to.")
	
	group.add_argument("--path", type=str, default=None, \
	                    help="Path to the JSON file with the controller configuration. \
	                          Ex: './coefficients.json'")
	
	# plot
	parser.add_argument('--ylims', type=list_of_floats, default=[20, 80], \
	                    help="minimum and maximum Limits of the Y axis (Celsius): [min, max]'")
	
	parser.add_argument('--window', type=float, default=120, \
	                    help="Width of time window shown in the X axis (Seconds)")

	parser.add_argument('--nsamples', type=positive_int, default=10, \
	                    help="Number of data samples used to initialize the plot. Must be \
	                         larger than zero")
	
	# object with arguments
	return parser.parse_args()

#===================================================================================================

def vert_array(arr: list) -> np.ndarray:
	'''Convert an horizontal array into a vertical numpy array. This function is well suited to
	transform parsed data into a form that is easily plotted by matplotlib. 
	
	Returns
	-------
	output : np.array
		Numpy array with the shape: [[]...[]]
	'''
	return np.array(arr).reshape(-1,1) 

#===================================================================================================

def main(port:str, path:str, window:float, ylims:list, nsamples:int):
	'''Interactive plot that displays temperature measurement in real time. Terminal can be used
	at the same time to send commands to the Arduino and modify the behavior of controller.
	
	Parameters
	----------
	port : str
		path to the serial port that the Arduino is connected to.
	
	path : str
		path to json file that contains the parameters to configure the controller.
	
	window : float
		Horizontal size of x-axis of the real time plot.
	
	ylims : list
		Range of visible temperatures in the real time plot; [minimum, maximum]
	
	nsamples : int
		number of data samples used to generate the first points in the plot.
	'''
	# ---- Serial ----
	controller = cntl.TempControllerCN0391(port=port, path=path)
	
	# ---- keyboard inputs ----
	keythread = kb.KeyboardThread() # starts thread automatically
	
	# ---- Plots ----
	# Size of data
	time_init = 0
	time.sleep(2)			# wait for serial to reload
	
	#print(controller.get_filter())
	y_init = sum( [vert_array(controller.get_filter()) for i in range(nsamples)] ) / nsamples
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
	ax.set_ylabel("Celsius")

	plt.ion()
	plt.show()
	
	#---- loop ----
	while controller.is_active(): 	# check connection
		try:
			# ----- serial inputs -----
			if keythread.flag():
				key_input = keythread.input()
				
				if key_input == "exit":
					break
				elif key_input != "":
					reply = controller.send_serial_command(key_input)
					print(reply)
			
			#----- plot -----
			time_now = time.time()
			
			if np.size(x) == 0: 
				time_init = time_now
				y = y_init.copy()
			else:
				ynew = controller.get_filter()	# breaks when interrupt is called
				ynew = vert_array(ynew)			# transform to vertical numpy array
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
			print("Data Transfer Failed - Exiting")
			break	# exit loop and ensure serial and thread shutdown
	
	#---- exit program ----
	print("CLOSED-PYTHON")
	controller.set_disable_all()
	controller.serial.reset()
	controller.serial.close()
	keythread.stop()

#===================================================================================================

if __name__ == '__main__':
	data = captureInputs()
	print(data)
	# capture serial data and plot it
	main(port=data.port,       \
	     path=data.path,       \
	     window=data.window,   \
	     ylims=data.ylims,     \
	     nsamples=data.nsamples)

