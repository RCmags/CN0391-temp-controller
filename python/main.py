# Global libraries
import matplotlib.pyplot as plt
import numpy as np
import time

# local file
import KeyboardThread as kb
import TempControllerCN0391 as cntl

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

def main(window, ylims, path=None, nsamples=10):
	'''Interactive plot that displays temperature measurement in real time. Terminal can be used
	at the same time to send commands to the Arduino and modify the behavior of controller.
	
	Parameters
	----------
	window : float
		Horizontal size of x-axis of the real time plot
	
	ylims : list
		Range of visible temperatures in the real time plot; [minimum, maximum]
	
	path : str
		path to json file that contains the parameters to configure the controller
	
	nsamples : int
		number of data samples used to generate the first points in the plot.
	'''
	# ---- Serial ----
	controller = cntl.TempControllerCN0391(path=path)
	
	# ---- keyboard inputs ----
	keythread = kb.KeyboardThread() # starts thread automatically
	
	# ---- Plots ----
	# Size of data
	time_init = 0
	time.sleep(2)			# wait for serial to reload
	
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
	ax.set_ylabel("Celcius")

	plt.ion()
	plt.show()
	
	#---- loop ----
	while controller.is_active(): 	# check connection
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
	
	#---- exit program ----
	print("CLOSED-PYTHON")
	controller.set_disable_all()
	controller.serial.reset()
	controller.serial.close()
	keythread.stop()

#===================================================================================================

if __name__ == '__main__':
	main(path='coefficients.json', window=120, ylims=[20, 80])


