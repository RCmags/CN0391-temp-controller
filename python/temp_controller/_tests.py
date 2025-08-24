# local import
import TempControllerCN0391 as cntl

# global import
import time
from pprint import pprint
import unittest
import argparse
from random import uniform

# Note: Turn into function that recieves baud rate, port, and other information.
# Use GUI app to procure this information.

# Note: Keep unit test logic encapsulated; 
# [+] User MUST specify port and baud rate; Will not work otherwise
# [+]wrap CLI inputs into functions; call in main function;
# [+]let main function be imported into other files as module 

#[EXTERNAL-INPUT]
"""
PORT="/dev/ttyACM0"
BAUD_RATE=9600
PATH_LOAD="./json/coefficients_load.json"
PATH_SAVE="./json/coefficients_save.json"
"""

#===================================================================================================

# [WORKING]
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


def random_float( nmin:float=0, nmax:float=1 ) -> float:
	num = uniform(nmin, nmax)
	num = round(num, cntl._DECIMAL_MAX) # Arduino uses float32, while python uses float64
	return num


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


def check_smaller( obj:object, inputs:list, outputs:list ) -> None:
	check = all( inputs[ch] > outputs[ch] for ch in range(0,4) )
	obj.assertTrue(check, "Sent input is not larger than output")


def print_results( inputs:list, outputs:list, isWarn=False, isSmaller=False ) -> None:
	print("\n### Net results:")
	print("Inputs:  ", inputs)
	print("Outputs: ", outputs)
	
	if isWarn:
		print("\n[WARNING] MUST MANUALLY INSPECT")
	
	if isSmaller:
		print("\n[NOTE] Outputs merely need to be smaller or similar to inputs")

#===================================================================================================

class LoadParameters(unittest.TestCase):
	
	# constants
	PORT = None
	BAUD_RATE = None
	PATH_LOAD = None
	PATH_SAVE = None
	
	@classmethod
	def constants(cls, port:str, baud_rate:int, path_load:str, path_save:str=None) -> None:
		cls.PORT = port
		cls.BAUD_RATE = baud_rate
		cls.PATH_LOAD = path_load
		cls.PATH_SAVE = path_save

#===================================================================================================

# [WORKING]
class TestConstructors(LoadParameters):
	
	def test_constructor_none(self):
		print("\n>>>> test_constructor_none")
		with self.assertRaises(Exception):
			controller = cntl.TempControllerCN0391()	# Ensure this raises an error
	
	#-----------------------------------------------------------------------------------------------
	
	def test_constructor_port(self):
		print("\n>>>> test_constructor_port")
		controller = cntl.TempControllerCN0391(port=self.PORT, baud_rate=self.BAUD_RATE) #[EXTERNAL-INPUT]
		controller.close()
	
	#-----------------------------------------------------------------------------------------------
	
	def test_constructor_path(self):
		print("\n>>>> test_constructor_path")
		controller = cntl.TempControllerCN0391(path=self.PATH_LOAD) #[EXTERNAL-INPUT]
		controller.close()

#===================================================================================================

class TestPythonAPI(LoadParameters):
	
	def test_getters(self):
		printSpacer()
		print("\n>>> Getters <<<")
		self.controller = cntl.TempControllerCN0391(path=self.PATH_LOAD) #[EXTERNAL-INPUT]
		
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
	
	#-----------------------------------------------------------------------------------------------
	
	def test_setters(self):
		printSpacer()
		print("\n<<< Setters >>>")
		self.controller = cntl.TempControllerCN0391(path=self.PATH_LOAD) # path does not define types
		
		printSpacer()
		
		with self.subTest():
			set_function_ch_random(self, "set_disable", num_input=0)
			inputs = [False, False, False, False]
			outputs = check_num_func(self, "get_enable", num_output=4)
			print_results(inputs, outputs)
			check_equality(self, inputs, outputs)
		
		printSpacer()
		
		# enable ports for subsequent tests
		with self.subTest():
			set_function_ch_random(self, "set_enable", num_input=0)
			inputs = [True, True, True, True]
			outputs = check_num_func(self, "get_enable", num_output=4)
			print_results(inputs, outputs)
			check_equality(self, inputs, outputs)
		
		printSpacer()
		
		with self.subTest():
			set_function_ch_random(self, "set_timeout_inf", num_input=0)
			inputs = [-1, -1, -1, -1]
			outputs = check_num_func(self, "get_timeout", num_output=4)
			print_results(inputs, outputs)
			check_equality(self, inputs, outputs)
		
		printSpacer()
		
		with self.subTest():
			inputs = [ random_float(nmin=0, nmax=100) for i in range(0, 4) ]
			set_function(self, "set_target_all", "", *inputs)
			
			outputs = check_num_func(self, "get_target", num_output=4)
			
			print_results(inputs, outputs)
			check_equality(self, inputs, outputs)
		
		printSpacer()
		
		# Single calls:
		with self.subTest():
			inputs = set_function_ch_random(self, "set_target", \
			                                      num_input=1, nmin=-50, nmax=500)
			
			outputs = check_num_func(self, "get_target", num_output=4)
			print_results(inputs, outputs)
			check_equality(self, inputs, outputs)
		
		printSpacer()
		
		with self.subTest():
			inputs = set_function_ch_random(self, "set_pid", \
			                                      num_input=3, nmin=0, nmax=100)
			
			outputs = check_num_func_channel(self, "get_pid", num_output=3) # multiple function calls
			print_results(inputs, outputs)
			check_equality(self, inputs, outputs)
		
		printSpacer()
		
		with self.subTest():
			inputs = set_function_ch_random(self, "set_ab_filter", \
			                                      num_input=2, nmin=0.1, nmax=1) # cannot be zero
			
			outputs = check_num_func_channel(self, "get_ab_filter", num_output=2) # multiple function calls
			print_results(inputs, outputs)
			check_equality(self, inputs, outputs)
		
		printSpacer()
		
		with self.subTest():
			inputs = set_function_ch_random(self, "set_k_filter", \
			                                      num_input=2, nmin=0.1, nmax=1) # cannot be zero 
			
			outputs = check_num_func_channel(self, "get_k_filter", num_output=2) # multiple function calls
			print_results(inputs, outputs)
			check_equality(self, inputs, outputs)
		
		printSpacer()
		
		with self.subTest():
			set_function_ch_random(self, "set_timeout_inf", num_input=0)
			outputs = check_num_func(self, "get_timeout", num_output=4)
			
			inputs = [-1, -1, -1 , -1] 				# See: "constants.h" for TIME_INF constant
			print_results(inputs, outputs)
			check_equality(self, inputs, outputs)
		
		printSpacer()
		
		with self.subTest(): # Had to fix arduino getter | prone to lack of ram errors
			inputs = set_function_ch_random(self, "set_in_limit", \
			                                      num_input=2, nmin=10, nmax=100)
			
			outputs = check_num_func_channel(self, "get_in_limit", num_output=2) 
			print_results(inputs, outputs, isWarn=True)
			
			# need to ensure inputs are different enough; cannot have nearly the same inputs
			IN_DIFF_MIN = 1 # defined in "arduino/src/PIDcontroller/PIDcontroller.cpp" 
			
			check = [ inputs[ch] == outputs[ch] or outputs[ch] == [IN_DIFF_MIN, 0] \
			          for ch in range(0,4) ]
			
			self.assertTrue(check, "Sent input does not match output or result in default output")
		
		printSpacer()
		
		with self.subTest(): # Timeout values can begin change if the number is too large. Beware.
			inputs = set_function_ch_random(self, "set_timeout", \
			                                      num_input=1, nmin=100000, nmax=1000000)
			
			outputs = check_num_func(self, "get_timeout", num_output=4)
			
			print_results(inputs, outputs, isWarn=True, isSmaller=True)
			print("\n[NOTE] Floats are not perfect representations of large numbers.")
			print("Inputs use float64 (Python) while Outputs (Arduino) use float32")
			
			# Comparison
			PERCENT = 0.1 
			check = [abs((inputs[ch] - outputs[ch])/outputs[ch]) < PERCENT for ch in range(0,4)]
			check = all(check)
			self.assertTrue(check, "Sent input is approximatley similar to the output")
		
		printSpacer()
		
		## Is this function useful? Needlessly biases sensor output
		with self.subTest(): 
			inputs = set_function_ch_random(self, "set_k_filter_state", \
			                                      num_input=1, nmin=10, nmax=100)
			
			outputs = check_num_func(self, "get_filter", num_output=4) 
			print_results(inputs, outputs, isWarn=True, isSmaller=True)
		
		printSpacer()
		
		#----- saving
		
		with self.subTest():
			print("<<<< Saved coefficients to: " + self.PATH_SAVE)
			self.controller.save_json_file(self.PATH_SAVE)
			data = self.controller.get_json_data()
			pprint(data, compact=True)
		
		# close connection gracefully
		with self.subTest():
			self.controller.set_disable_all()
			self.controller.close()
			print("\n>>>> CLOSED CONNECTION <<<<")

#===================================================================================================

def captureInputs() -> object:
	parser = argparse.ArgumentParser()
	
	# Inputs
	parser.add_argument('--port', type=str, required=True, \
	                    help="Port where the Arduino is connected")
	
	parser.add_argument('--baud_rate', type=int, required=True, \
	                    help="Baud rate at which the Arduino's serial is configured")
	
	parser.add_argument('--path_load', type=str, \
	                    default="./json/coefficients_load.json", \
	                    help="Path where json configuration file is located")
	
	parser.add_argument('--path_save', type=str, \
	                    default="./json/coefficients_save.json", \
	                    help="Path where the json file for testing will be saved")
	
	# Outputs
	#return parser.parse_args()
	return parser.parse_known_args()


def testAPI(port:str, baud_rate:int, path_load:str, path_save:str, unittest_args:object) -> None:
	# configure
	TestConstructors.constants(port, baud_rate, path_load)
	TestPythonAPI.constants(port, baud_rate, path_load, path_save)
	# Run tests
	_argv = [''] + unittest_args
	unittest.main( argv=_argv )

#===================================================================================================

if __name__ == '__main__':
	args, unittest_args = captureInputs()
	
	testAPI(args.port,      \
	        args.baud_rate, \
	        args.path_load, \
	        args.path_save, \
	        unittest_args)


