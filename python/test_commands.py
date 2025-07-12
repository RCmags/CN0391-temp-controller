import TempControllerCN0391 as cntl

PORT 	  = "/dev/ttyACM0"	# String of the Serial port used by the arduino
BAUD_RATE = 9600
controller = cntl.TempControllerCN0391(PORT, BAUD_RATE)
controller.setup()

# Note: come up with more demanding test routine. Continuous getters or setters.
# then check results. Loops of data retrieval, etc. 

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

exit()
