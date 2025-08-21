# local import
import TempControllerCN0391 as cntl

# global import
import time
from pprint import pprint
import unittest
import argparse
from random import random


# Note: Turn into function that recieves baud rate, port, and other information.
# Use GUI app to procure this information.

#===================================================================================================

# Note: Keep unit test logic encapsulated; 
# User MUST specify port and baud rate; Will not work otherwise
# wrap CLI inputs into functions; call in main function;
# let main function be imported into other files as module 

""" [UNUSED]
parser = argparse.ArgumentParser()

parser.add_argument('--port', type=str, \
	                 help="Path where json configuration file is located")

parser.add_argument('--baud_rate', type=str, \
	                 help="Path where json configuration file is located")

parser.add_argument('--json_load_path', type=str, \
	                 help="Path where json configuration file is located")

parser.add_argument('--json_save_path', type=str, \
	                 help="Path where the json file for testing will be saved")

cmds = parser.parse_args()
"""

PORT="/dev/ttyACM0"
BAUD_RATE=9600
PATH_LOAD="./json/coefficients_load.json"
PATH_SAVE="./json/coefficients_save.json"

#===================================================================================================

# Prevent random test order
#unittest.TestLoader.sortTestMethodsUsing = None

""" [WORKING]
class TestConstructors(unittest.TestCase):

	def test_constructor_none(self):
		print("\n>> test_constructor_none")
		with self.assertRaises(Exception):
			controller = cntl.TempControllerCN0391()
	
	#-----------------------------------------------------------------------------------------------
	
	def test_constructor_port(self):
		print("\n>> test_constructor_port")
		controller = cntl.TempControllerCN0391(port=PORT, baud_rate=BAUD_RATE)
		controller.close()
	
	#-----------------------------------------------------------------------------------------------
	
	def test_constructor_path(self):
		print("\n>> test_constructor_path")
		controller = cntl.TempControllerCN0391(path=PATH_LOAD)
		controller.close()
"""

#===================================================================================================

def check_num_array( data:list, length:int ) -> bool:
	check = len(data) == length and \
	        all( isinstance(num, float) or isinstance(num, int) for num in data )
	return check


def check_num_func( obj:object, func:str, num_output:int, msg:str="", ch:int=None ) -> list:
	print("\n>>>> " + func + msg)
	
	# select correct function
	if type(ch) == int:
		data = getattr(obj.controller, func)(ch)
	else:
		data = getattr(obj.controller, func)()
	print(data)
	
	# check output types
	check = check_num_array(data, num_output)
	obj.assertTrue( check, "Not all elements are numbers" )
	
	return data


def check_num_func_channel( obj:object, func:str, num_output:int ) -> list:
	output = []
	
	for ch in range(0, 4):
		msg = ": ch" + str(ch)
		data = check_num_func(obj, func, num_output, msg, ch=ch)
		output.append(data)
	
	return output


def check_sensor_type( obj:object ) -> list:
	print("\n>>>> get_sensor_type")
	data = obj.controller.get_sensor_type()
	check = [isinstance(data, str) and len(elem) == 1 for elem in data]
	obj.assertTrue(check, "Unacceptable sensor types. They are not single letters")
	return list(data)


def printSpacer():
	print("\n---------------------")

#---------------------------------------------------------------------------------------------------

# NOTE: need to send known data, then get said data, and check if its the same.

def random_float( nmin:float=0, nmax:float=1 ) -> float:
	return round(nmin + random()*nmax, cntl._DECIMAL_MAX)


def set_function( obj:object, func:str, msg:str="", *args ) -> None:
	print("\n<<<< " + func + msg)
	print(args)
	getattr(obj.controller, func)(*args) 	# Call setter function


def set_function_ch_random( obj:object, func:str, num_input:int, \
                                        nmin:float=0, nmax:float=1 ) -> None:
	output = []
	
	for ch in range(0, 4):
		inputs = [ random_float(nmin, nmax) for i in range(0, num_input) ]
		
		if len(inputs) == 1:
			output.append(*inputs)
		else:
			output.append(inputs)
		
		inputs = [ch, *inputs]
		msg = ": ch" + str(ch)
		set_function(obj, func, msg, *inputs)
	
	return output


def check_equality( obj:object, inputs:list, outputs:list ) -> None:
	check = outputs == inputs
	obj.assertTrue(check, "Sent input and received output do not match")


def print_results( inputs:list, outputs:list, isWarn=False ) -> None:
	print("\n### Net results:")
	
	if isWarn:
		print("[WARNING] MUST MANUALLY INSPECT")
	
	print("Inputs:  ", inputs)
	print("Outputs: ", outputs)


#---------------------------------------------------------------------------------------------------

class TestPythonAPI(unittest.TestCase):
	
	def setUp(self):
		print("\n>> test_python_api")
	
	"""
	def test_getters(self):
		print(">>> Getters <<<")
		self.controller = cntl.TempControllerCN0391(path=PATH_LOAD)
		
		printSpacer()
		
		# Single calls:
		with self.subTest():
			check_num_func(self, "get_filter", 4)
		
		with self.subTest():
			check_num_func(self, "get_raw", 4)
		
		with self.subTest():
			check_num_func(self, "get_target", 4)
		
		with self.subTest():
			check_num_func(self, "get_timeout", 4)
		
		printSpacer()
		
			# Non-numeric:
		with self.subTest():
			check_num_func(self, "get_enable", 4) # checks boolean
		
		with self.subTest():
			check_sensor_type(self)               # checks string
		
		printSpacer()
		
		# Per channel calls:
		with self.subTest():
			check_num_func_channel(self, "get_pid", 3)
		
		printSpacer()
		
		with self.subTest():
			check_num_func_channel(self, "get_in_limit", 2)
		
		printSpacer()
		
		with self.subTest():
			check_num_func_channel(self, "get_ab_filter", 2)
		
		printSpacer()
		
		with self.subTest():
			check_num_func_channel(self, "get_k_filter", 2)
		
		# close connection gracefully
		self.controller.close()
	"""
	
	#-----------------------------------------------------------------------------------------------
	
	def test_setters(self):
		print("<<< Setters >>>")
		self.controller = cntl.TempControllerCN0391(path=PATH_LOAD)
		
		printSpacer()
		
		# Single calls:
		with self.subTest():
			inputs = set_function_ch_random(self, "set_target", 1)
			outputs = check_num_func(self, "get_target", 4)
			print_results(inputs, outputs)
			check_equality(self, inputs, outputs)
		
		printSpacer()
		
		with self.subTest():
			inputs = set_function_ch_random(self, "set_pid", 3)
			outputs = check_num_func_channel(self, "get_pid", 3) # multiple function calls
			print_results(inputs, outputs)
			check_equality(self, inputs, outputs)
		
		printSpacer()
		
		with self.subTest(): # Had to fix arduino getter | prone to lack of ram errors
			inputs = set_function_ch_random(self, "set_in_limit", 2, nmin=10, nmax=100)
			outputs = check_num_func_channel(self, "get_in_limit", 2) 
			print_results(inputs, outputs, isWarn=True)
			check_equality(self, inputs, outputs)
		
		printSpacer()
		
		with self.subTest():
			inputs = set_function_ch_random(self, "set_ab_filter", 2)
			outputs = check_num_func_channel(self, "get_ab_filter", 2) # multiple function calls
			print_results(inputs, outputs)
			check_equality(self, inputs, outputs)
		
		printSpacer()
		
		with self.subTest():
			inputs = set_function_ch_random(self, "set_k_filter", 2)
			outputs = check_num_func_channel(self, "get_k_filter", 2) # multiple function calls
			print_results(inputs, outputs)
			check_equality(self, inputs, outputs)
		
		printSpacer()
		
		with self.subTest():
			inputs = set_function_ch_random(self, "set_timeout", 1)
			outputs = check_num_func(self, "get_timeout", 4)
			print_results(inputs, outputs)
			check_equality(self, inputs, outputs)
		
		printSpacer()
		
		## Manual check
		with self.subTest(): ## This just needs to change the output. 
			inputs = set_function_ch_random(self, "set_k_filter_state", 1, nmin=10, nmax=100)
			outputs = check_num_func(self, "get_filter", 4) 
			print_results(inputs, outputs, isWarn=True)
		
		printSpacer()
		
		# close connection gracefully
		self.controller.close()
	
	"""
	# by channel
	[+]controller.set_target(0, 45)			-> random floats
	[+]controller.set_pid(0, 4, 2, 3)			-> random floats
	controller.set_in_limit(0, 50, 25)		-> random floats
	[+]controller.set_ab_filter(0, 0.1, 0.2)	-> random floats
	[+]controller.set_k_filter(0, 0.3, 0.4)	-> random floats
	controller.set_k_filter_state(0, 100)	-> random floats
	[+]controller.set_timeout(0, 100)			-> random floats
	
		# no parameters besides: ch
	controller.set_enable(0)
	controller.set_disable(0)
	controller.set_timeout_inf(0)
	
	# no channel
	controller.set_target_all(10, 20, 30, 40) -> random floats
	
	# no input
	controller.set_enable_all()
	controller.set_disable_all()
	"""

#===================================================================================================

if __name__ == '__main__':
	unittest.main()
	#turn off serial

#------------------

# NOTE: convert code and command line option to "main.py"

#===================================================================================================


""" TARGET EXAMPLE
inputs = set_function_ch_random(self, "set_target", 1)
outputs = check_num_func(self, "get_target", 4) ## Singular function call, no channel input
print_results(inputs, outputs)
check_equality(self, inputs, outputs)
"""

""" PID EXAMPLE
inputs = set_function_ch_random(self, "set_pid", 3)
outputs = check_num_func_channel(self, "get_pid", 3) ## multiple function calls
print_results(inputs, outputs)
check_equality(self, inputs, outputs)
"""

"""
with self.subTest():
--arrays--
	[+]get_filter()
	[+]get_raw()
	[+]print( controller.get_target() )
	[+]print( controller.get_timeout() )


--array|no_number--
	print( controller.get_sensor_type() ) # string characters
	[+]print( controller.get_enable() )  # bool -> works as int

--array|per channel--
	[+]print( controller.get_pid(1) )
	[+]print( controller.get_in_limit(1) )
	[+]print( controller.get_ab_filter(1) )
	[+]print( controller.get_k_filter(1) )
"""

"""

controller = cntl.TempControllerCN0391(port=PORT, baud_rate=BAUD_RATE) # test1 -> must load constants.h -> does?
#controller = cntl.TempControllerCN0391() # test2
#controller = cntl.TempControllerCN0391(path="coefficients.json") # test3

#--- test functions ---
print("===========TEST-1")

print( controller.get_filter() )
print( controller.get_raw() )

***controller.set_target(1, 45)
print( controller.get_target() )

***controller.set_target_all(10, 20, 30, 40)
print( controller.get_target() )

***controller.set_pid(0, 4, 2, 3)
controller.set_pid(1, 5, 6, 7)
controller.set_pid(3, 8, 9, 10)
print( controller.get_pid(0) )

***controller.set_in_limit(0, 50, 25)
print( controller.get_in_limit(0) )

***controller.set_ab_filter(0, 0.1, 0.2)
print( controller.get_ab_filter(0) )

***controller.set_k_filter(0, 0.3, 0.4)
print( controller.get_k_filter(0) )

***controller.set_k_filter_state(0, 100)
print( controller.get_filter() )

print( controller.get_sensor_type() )

***controller.set_enable(0)
print( controller.get_enable() )

***controller.set_enable_all()
print( controller.get_enable() )

***controller.set_disable(0)
print( controller.get_enable() )

***controller.set_disable_all()
print( controller.get_enable() )

print( controller.get_timer() )

***controller.set_timeout(0, 100)
print( controller.get_timeout() )

***controller.set_timeout_inf(0)
print( controller.get_timeout() )

#==================================================
print("===========TEST-2")

controller.set_target(0, 45)
controller.set_target_all(10, 20, 30, 40)
controller.set_pid(0, 4, 2, 3)
controller.set_in_limit(0, 50, 25)
controller.set_ab_filter(0, 0.1, 0.2)
controller.set_k_filter(0, 0.3, 0.4)
controller.set_k_filter_state(0, 100)
controller.set_enable(0)
controller.set_enable_all()
controller.set_disable(0)
controller.set_disable_all()
controller.set_timeout(0, 100)
controller.set_timeout_inf(0)
controller.set_enable_all()
controller.set_timeout(2, 100)
controller.set_k_filter(1, 0.35, 0.412)

#time.sleep(10)
print( controller.get_filter() )
print( controller.get_raw() )
print( controller.get_target() )
print( controller.get_pid(1) )
print( controller.get_in_limit(1) )
print( controller.get_ab_filter(1) )
print( controller.get_k_filter(1) )
print( controller.get_sensor_type() )
print( controller.get_enable() )
print( controller.get_timeout() )

#==================================================
print("===========TEST-3")

controller.save_json_file('../coefficients_test.json')
pprint.pprint(controller.get_json_data(), compact=True)

#--------------------- quit
controller.set_disable_all()
controller.close()
exit()


"""

