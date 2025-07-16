import TempControllerCN0391 as cntl
import time

PORT 	  = "/dev/ttyACM0"	# String of the Serial port used by the arduino
BAUD_RATE = 9600
controller = cntl.TempControllerCN0391(PORT, BAUD_RATE)
controller.setup('N', 'K', 'J', 'K')

# Note: come up with more demanding test routine. Continuous getters or setters.
# then check results. Loops of data retrieval, etc. 


#--- test functions ---
print("===========TEST-1")

print( controller.get_filter() )
print( controller.get_raw() )

controller.set_target(1, 45)
print( controller.get_target() )

controller.set_target_all(10, 20, 30, 40)
print( controller.get_target() )

controller.set_pid(0, 4, 2, 3)
controller.set_pid(1, 5, 6, 7)
controller.set_pid(3, 8, 9, 10)
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
print( controller.get_target() )


print( controller.get_pid(0) )


print( controller.get_in_limit(0) )
print( controller.get_ab_filter(0) )
print( controller.get_k_filter(0) )
print( controller.get_filter() )
print( controller.get_sensor_type() )
print( controller.get_enable() )
print( controller.get_timeout() )


#==================================================
print("===========TEST-3")
controller.get_device_coefficients()
controller.save_json_file('save_const.json')
print( controller.get_json_data() )


#--------------------- quit
controller.serial.reset()
controller.close()
exit()
