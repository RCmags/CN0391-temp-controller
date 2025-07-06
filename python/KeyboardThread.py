import threading

#=========================================================================================

# Class to get keyboard inputs via non-blocking function
# See: 	https://stackoverflow.com/questions/2408560/non-blocking-console-input
#		https://www.geeksforgeeks.org/python/python-different-ways-to-kill-a-thread/

class KeyboardThread(threading.Thread):
	
	def __init__(self, function = None, name='keyboard-input-thread'):
		super(KeyboardThread, self).__init__(name=name, daemon=True)
		self._function = function
		self._stop = threading.Event()
		self._input_data = ""					# initialize to empty string
	
	def run(self):
		while self.is_active():
			self._input_data = input()
			self._function( self._input_data )	   #waits to get input + Return
	
	def input(self):
		return self._input_data
	
	def stop(self):
		self._stop.set()
	
	def is_active(self):
		return not self._stop.isSet()
	
	"""
	def start(self): # begin thread
	"""

