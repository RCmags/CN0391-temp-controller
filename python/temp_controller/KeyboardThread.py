"""
Module intended to asynchronously capture user inputs without blocking the execution of other code.
It was designed for the purpose of sending serial commands in real time to an Arduino. 
"""

import threading
from typing import Callable

#=========================================================================================

# Class to get keyboard inputs via non-blocking function
# See: 	https://stackoverflow.com/questions/2408560/non-blocking-console-input
#		https://www.geeksforgeeks.org/python/python-different-ways-to-kill-a-thread/

class KeyboardThread(threading.Thread):
	
	def __init__(self, function:Callable = None, name:str = 'keyboard-input-thread'):
		'''Initialize thread to receive keyboard inputs
		
		Parameters
		----------
		function: func
			Callback function that is execution whenever the user provides a keyboard input.
			Must have the form `def function(string)` that takes a string 
			(which is the user input) and performs some process. 
 		
		name: str
			label for the thread. If none is given it defaults to a generic name. 
		'''
		super(KeyboardThread, self).__init__(name=name, daemon=True)
		# variables
		self._function = function
		self._stop = threading.Event()
		self._input_data = ""					# initialize to empty string
		self._flag = False
		# initialize thread
		self.start()
	
	def run(self) -> None:
		'''Loop that is executed by the thread that listens to keyboard inputs.
		This loop will continue indefinately until `stop()` is called. 
		'''
		while self.is_active():
			# get input
			self._input_data = input()
			if self._input_data != "":
				self._flag = True	# data is unread
			# callback
			if self._function != None:
				self._function( self._input_data ) #waits to get input + Return
	
	
	def setCallback(self, function:Callable) -> None:
		''' Set the callback function that is called whenever the user submits an input
		
		Parameters
		----------
		function: func
			Callback function. See description of the class initializer. 
		'''
		self._function = function
	
	
	def input(self) -> str:
		'''Returns the keyboard input provided by the user. Updates whenever the user presses enter
		
		Returns
		-------
		input: str
		'''
		self._flag = False			# data was read
		return self._input_data
	
	
	def flag(self) -> bool:
		'''Flag that indicates whether a new keyboard input has been provided and is unread.
		
		Returns
		-------
		flag: bool
			`True` if a new input was provided and input() has been called. 
			`False` if no new input is available and input() has beed called.
		'''
		return self._flag
	
	
	def stop(self) -> None:
		'''Kills the thread focus on receiving keyboard inputs. Must be called whenever the
		program has finished to prevent the thread from blocking the program from closing. 
		'''
		self._stop.set()
	
	
	def is_active(self) -> bool:
		'''Indicates whether the thread is active. 
		
		Returns
		-------
		flag: bool
			True when the thread is active and listening to inputs
			False when `stop()` is called and the thread is closed. 
		'''
		return not self._stop.isSet()
	
	
	#def start(self): # begin thread
		'''Function that starts the thread. 
		This function is called automatically when the class is initialized. It allows
		`input()` to update.
		'''

