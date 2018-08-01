import megaio as m
import time

total_event = 10;

def opto_callback(ch):
  global total_event
  print ("Event on ch", ch)
  total_event-= 1
  print (total_event, "events remaining")
  if total_event == 0:
    m.remove_all_opto_events();
    print ("Remove  all events, exiting..")
    exit()
	
m.add_opto_event(0, 1, m.RISING, opto_callback)
m.add_opto_event(0, 2, m.RISING, opto_callback)

try:
  print ("This example show you how to use external interrupt on megaio board")
  
  print ("Connect pin 2 and 4 for optocuplers power(+5V and 5VEXT)")
  print ("Momentary connect opto channel 1 and 2 (pin 3 and 5) to GND")
  print ("for generating the interrupts")
  print ("Waith for opto events..")
  while 1:
    time.sleep(.02)
  print ("end wait")
  
except KeyboardInterrupt:  
  print ("Interrupted")
