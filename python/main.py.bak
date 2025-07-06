# Global libraries
import matplotlib.pyplot as plt
import numpy as np
import time

# local file
import ArduinoSerial

#=========================================================================================

def main(port, baud_rate, window, ylims, nsamples=10):
	# ---- State ----
	# Serial 
	serialCom = ArduinoSerial.SerialCommunication(port, baud_rate)

	# size of data
	time_init = 0
	time.sleep(2)								# wait for serial to reload
	y_init = serialCom.averageSignal(nsamples)
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
	while True:
		if not serialCom.getFlag():
			serialCom.close()		# close serial com
			plt.close(figure) 		# close figure
			break
		
		# update plot data
		time_now = time.time()

		if np.size(x) == 0: 
			time_init = time_now
			y = y_init.copy()
		else:
			ynew = serialCom.readData()
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

#=========================================================================================

if __name__ == '__main__':
	# Inputs
	PORT 		= "/dev/ttyACM0"	# String of the Serial port used by the arduino
	BAUD_RATE 	= 9600				# Baud rate used by the arduino. Must match the value in "Constants.h"
	X_WINDOW 	= 120				# Horizontal size of x-axis of the real time plot
	Y_LIMS 		= [20, 80]			# Range of visible temperatures in the real time plot; [minimum, maximum]
	main( PORT, BAUD_RATE, X_WINDOW, Y_LIMS )
	

# Note: the Serial port changes with operating system:
# 		On Linux and macOS, use '/dev/ttyACM0' or similar
#		On Windows, use 'COM3' or similar

