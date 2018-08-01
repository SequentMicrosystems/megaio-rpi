import smbus
import RPi.GPIO as GPIO 

#bus = smbus.SMBus(1)    # 0 = /dev/i2c-0 (port I2C0), 1 = /dev/i2c-1 (port I2C1)

DEVICE_ADDRESS = 0x31     #7 bit address (will be left shifted to add the read write bit)
RETRY_TIMES	= 10


RELAY_MEM_ADD = 0x00
RELAY_ON_MEM_ADD = 0x01
RELAY_OFF_MEM_ADD = 0x02
OPTO_IN_MEM_ADD = 0x03
OC_OUT_VAL_MEM_ADD = 0x04
OC_OUT_SET_MEM_ADD = 0x05
OC_OUT_CLR_MEM_ADD = 0x06

ADC_VAL_MEM_ADD = 0x07
DAC_VAL_H_MEM_ADD = 0x17
DAC_VAL_L_MEM_ADD = 0x18
GPIO_VAL_MEM_ADD = 0x19
GPIO_SET_MEM_ADD = 0x1a
GPIO_CLR_MEM_ADD = 0x1b
GPIO_DIR_MEM_ADD = 0x1c
OPTO_IT_RISING_MEM_ADD = 0x1d # 1B
OPTO_IT_FALLING_MEM_ADD = 0x1e # 1B
GPIO_EXT_IT_RISING_MEM_ADD = 0x1f # 1B
GPIO_EXT_IT_FALLING_MEM_ADD = 0x20 # 1B
OPTO_IT_FLAGS_MEM_ADD = 0x21 # 1B
GPIO_IT_FLAGS_MEM_ADD = 0x22 # 1B

REVISION_HW_MAJOR_MEM_ADD = 0x3c
REVISION_HW_MINOR_MEM_ADD = 0x3d
REVISION_MAJOR_MEM_ADD = 0x3e
REVISION_MINOR_MEM_ADD = 0x3f

RISING = 1
FALLING = 0

opto_callbacks = {}
gpio_callbacks = {}

GPIO.setmode(GPIO.BCM) 
GPIO.setup(4, GPIO.IN, pull_up_down=GPIO.PUD_UP)  


def version():
  print ("megaio python library v1.2 Sequent Microsystems")
  
def set_relay(stack, relay, value):
	bus = smbus.SMBus(1)
	if stack < 0 or stack > 3:
		raise ValueError('Invalid stack level')
		return
	if relay < 0:
		raise ValueError('Invalid relay number')
		return
	if relay > 8:
		raise ValueError('Invalid relay number')
		return
	if value == 0:
		bus.write_byte_data(DEVICE_ADDRESS + stack, RELAY_OFF_MEM_ADD, relay)
	else:
		bus.write_byte_data(DEVICE_ADDRESS + stack, RELAY_ON_MEM_ADD, relay)

		
def set_relays(stack, value):
	bus = smbus.SMBus(1)
	if stack < 0 or stack > 3:
		raise ValueError('Invalid stack level')
		return
	if value > 255 :
		raise ValueError('Invalid relay value')
		return
	if value < 0:
		raise ValueError('Invalid relay value')
		return
	bus.write_byte_data(DEVICE_ADDRESS + stack, RELAY_MEM_ADD, value)
	
	
def get_relays(stack):
	bus = smbus.SMBus(1)
	if stack < 0 or stack > 3:
		raise ValueError('Invalid stack level')
		return
	return bus.read_byte_data(DEVICE_ADDRESS + stack, RELAY_MEM_ADD)
	
def get_adc(stack, channel):
	outData = 0
	aux = 0
	bus = smbus.SMBus(1)
	if stack < 0 or stack > 3:
		raise ValueError('Invalid stack level')
		return
	if channel < 1 or channel > 8:
		raise ValueError('Invalid channel number')
		return
	aux = bus.read_word_data(DEVICE_ADDRESS + stack, ADC_VAL_MEM_ADD + 2*(channel - 1))
	outData = (0xff00 & (aux << 8)) + ( 0xff & (aux >> 8))
	return outData
	
def get_adc_volt(stack, channel):
	outData = 0
	aux = 0
	bus = smbus.SMBus(1)
	if stack < 0 or stack > 3:
		raise ValueError('Invalid stack level')
		return
	if channel < 1 or channel > 8:
		raise ValueError('Invalid channel number')
		return
	aux = bus.read_word_data(DEVICE_ADDRESS + stack, ADC_VAL_MEM_ADD + 2*(channel - 1))
	outData = (0xff00 & (aux << 8)) + ( 0xff & (aux >> 8))
	outData = 3.3 * outData / 4096;
	return outData
	
def set_dac(stack, value):
	outData = 0
	bus = smbus.SMBus(1)
	if stack < 0 or stack > 3:
		raise ValueError('Invalid stack level')
		return
	if value < 0 or value > 4095:
		raise ValueError('Invalid DAC value')
		return
	outData = (0xff00 & (value << 8)) + ( 0xff & (value >> 8))
	bus.write_word_data(DEVICE_ADDRESS + stack, DAC_VAL_H_MEM_ADD, outData)
	return
	
def set_dac_volt(stack, value):
	outData = 0
	ival = 0;
	bus = smbus.SMBus(1)
	if stack < 0 or stack > 3:
		raise ValueError('Invalid stack level')
		return
	if value < 0 or value > 3.3:
		raise ValueError('Invalid DAC value')
		return
	ival = int(4095 * value/3.3);
	outData = (0xff00 & (ival << 8)) + ( 0xff & (ival >> 8))
	bus.write_word_data(DEVICE_ADDRESS + stack, DAC_VAL_H_MEM_ADD, outData)
	return
	
def get_dac(stack):
	outData = 0
	aux = 0
	bus = smbus.SMBus(1)
	if stack < 0 or stack > 3:
		raise ValueError('Invalid stack level')
		return
	aux = bus.read_word_data(DEVICE_ADDRESS + stack, DAC_VAL_H_MEM_ADD)
	outData = (0xff00 & (aux << 8)) + ( 0xff & (aux >> 8))
	return outData	
	
def get_opto_in(stack):
	outData = 0
	bus = smbus.SMBus(1)
	if stack < 0 or stack > 3:
		raise ValueError('Invalid stack level')
		return
	outData = bus.read_byte_data(DEVICE_ADDRESS + stack, OPTO_IN_MEM_ADD)
	return outData
	
def set_io_pin_dir(stack, pin, dir):
	bus = smbus.SMBus(1)
	if stack < 0 or stack > 3:
		raise ValueError('Invalid stack level')
		return
	if pin < 1 or pin > 6:
		raise ValueError('Invalid IO pin number')
		return
	if dir != 0 and dir != 1:
		raise ValueError('Invalid IO direction')
		return
	inData = bus.read_byte_data(DEVICE_ADDRESS + stack, GPIO_DIR_MEM_ADD)
	aux = 1 << (pin-1);
	if dir == 1:
		inData = inData | aux
	else:
		inData = inData & (~aux)
	bus.write_byte_data(DEVICE_ADDRESS + stack, GPIO_DIR_MEM_ADD, inData)
	return
	
def get_io_val(stack):
	bus = smbus.SMBus(1)
	if stack < 0 or stack > 3:
		raise ValueError('Invalid stack level')
		return
	outVal = bus.read_byte_data(DEVICE_ADDRESS + stack, GPIO_VAL_MEM_ADD)
	return outVal
	
def set_io_pin(stack, pin, val):
	bus = smbus.SMBus(1)
	if stack < 0 or stack > 3:
		raise ValueError('Invalid stack level')
		return
	if pin < 1 or pin > 6:
		raise ValueError('Invalid IO pin number')
		return
	if val != 0 and val != 1:
		raise ValueError('Invalid IO output level')
		return
	if val == 1:	
		bus.write_byte_data(DEVICE_ADDRESS + stack, GPIO_SET_MEM_ADD, val)	
	else:
		bus.write_byte_data(DEVICE_ADDRESS + stack, GPIO_CLR_MEM_ADD, val)	
	return

def set_oc_pin(stack, pin, val):
	bus = smbus.SMBus(1)
	if stack < 0 or stack > 3:
		raise ValueError('Invalid stack level')
		return
	if pin < 1 or pin > 4:
		raise ValueError('Invalid OC channel number')
		return
	if val != 0 and val != 1:
		raise ValueError('Invalid OC value')
		return
	if val == 1:	
		bus.write_byte_data(DEVICE_ADDRESS + stack, OC_OUT_SET_MEM_ADD, val)	
	else:
		bus.write_byte_data(DEVICE_ADDRESS + stack, OC_OUT_CLR_MEM_ADD, val)	
	return
	
def get_oc_val(stack):
	bus = smbus.SMBus(1)
	if stack < 0 or stack > 3:
		raise ValueError('Invalid stack level')
		return
	outVal = bus.read_byte_data(DEVICE_ADDRESS + stack, OC_OUT_VAL_MEM_ADD)
	return outVal
	
#External interrupt setup functions ###########################################

#add optocupled input event
def add_opto_event(stack, ch, edge, callback):
  global opto_callbacks
  if stack < 0 or stack > 3:
    raise ValueError('Invalid stack level')
    return

  if edge != RISING and edge != FALLING or ch < 1 or ch > 8:
    raise ValueError('Invalid edge or channel')
    return
    
  if stack not in opto_callbacks:
    opto_callbacks[stack] = {}  
  if ch not in opto_callbacks[stack]:
    opto_callbacks[stack][ch] = {}
    
  opto_callbacks[stack][ch][edge] = callback
  
  bus = smbus.SMBus(1)
  aux = 1 << (ch-1); 
  if edge == RISING: 
    inData = bus.read_byte_data(DEVICE_ADDRESS + stack, OPTO_IT_RISING_MEM_ADD)
    inData = inData | aux;
    bus.write_byte_data(DEVICE_ADDRESS + stack, OPTO_IT_RISING_MEM_ADD, inData)
  else:
    inData = bus.read_byte_data(DEVICE_ADDRESS + stack, OPTO_IT_FALLING_MEM_ADD)
    inData = inData | aux;
    bus.write_byte_data(DEVICE_ADDRESS + stack, OPTO_IT_FALLING_MEM_ADD, inData)  

# remove optocupled input event
def remove_opto_event(stack, ch, edge):
  global opto_callbacks
  if stack < 0 or stack > 3:
    raise ValueError('Invalid stack level')
    return
      
  if edge != RISING and edge != FALLING or ch < 1 or ch > 8:
    raise ValueError('Invalid edge or channel')
    return
  if stack in opto_callbacks: 
    if ch in opto_callbacks[stack]:
      if edge in opto_callbacks[stack][ch]:
        del opto_callbacks[stack][ch][edge]
      
  bus = smbus.SMBus(1)
  aux = ~(1 << (ch-1)); 
  if edge == RISING: 
    inData = bus.read_byte_data(DEVICE_ADDRESS + stack, OPTO_IT_RISING_MEM_ADD)
    inData = inData & aux;
    bus.write_byte_data(DEVICE_ADDRESS + stack, OPTO_IT_RISING_MEM_ADD, inData)
  else:
    inData = bus.read_byte_data(DEVICE_ADDRESS + stack, OPTO_IT_FALLING_MEM_ADD)
    inData = inData & aux;
    bus.write_byte_data(DEVICE_ADDRESS + stack, OPTO_IT_FALLING_MEM_ADD, inData) 
     
# remove all optocupled input events  
def remove_all_opto_events():
  global opto_callbacks
  for stack in opto_callbacks:
    for ch in opto_callbacks[stack]:
      remove_opto_event(stack, ch, RISING)
      remove_opto_event(stack, ch, FALLING)
  
  del opto_callbacks
  
  
 #add gpio input event
def add_gpio_event(stack, ch, edge, callback):
  global gpio_callbacks
  
  if stack < 0 or stack > 3:
    raise ValueError('Invalid stack level')
    return 
       
  if edge != RISING and edge != FALLING or ch < 1 or ch > 6:
    raise ValueError('Invalid edge or channel')
    return
    
  if stack not in gpio_callbacks:
     gpio_callbacks[stack] = {}    
    
  if ch not in gpio_callbacks[stack]:
    gpio_callbacks[stack][ch] = {}
    
  gpio_callbacks[stack][ch][edge] = callback
  
  bus = smbus.SMBus(1)
  aux = 1 << (ch-1); 
  if edge == RISING: 
    inData = bus.read_byte_data(DEVICE_ADDRESS + stack, GPIO_EXT_IT_RISING_MEM_ADD)
    inData = inData | aux;
    bus.write_byte_data(DEVICE_ADDRESS + stack, GPIO_EXT_IT_RISING_MEM_ADD, inData)
  else:
    inData = bus.read_byte_data(DEVICE_ADDRESS + stack, GPIO_EXT_IT_FALLING_MEM_ADD)
    inData = inData | aux;
    bus.write_byte_data(DEVICE_ADDRESS + stack, GPIO_EXT_IT_FALLING_MEM_ADD, inData)  

# remove gpio input event
def remove_gpio_event(stack, ch, edge):
  global gpio_callbacks
  
  if stack < 0 or stack > 3:
    raise ValueError('Invalid stack level')
    return
      
  if edge != RISING and edge != FALLING or ch < 1 or ch > 6:
    raise ValueError('Invalid edge or channel')
    return
    
  if stack in gpio_callbacks: 
    if ch in gpio_callbacks[stack]:
      if edge in gpio_callbacks[stack][ch]:
        del gpio_callbacks[stack][ch][edge]
      
  bus = smbus.SMBus(1)
  aux = ~(1 << (ch-1)); 
  if edge == RISING: 
    inData = bus.read_byte_data(DEVICE_ADDRESS + stack,  GPIO_EXT_IT_RISING_MEM_ADD)
    inData = inData & aux;
    bus.write_byte_data(DEVICE_ADDRESS + stack,  GPIO_EXT_IT_RISING_MEM_ADD, inData)
  else:
    inData = bus.read_byte_data(DEVICE_ADDRESS + stack,  GPIO_EXT_IT_FALLING_MEM_ADD)
    inData = inData & aux;
    bus.write_byte_data(DEVICE_ADDRESS + stack,  GPIO_EXT_IT_FALLING_MEM_ADD, inData)  
 
# remove all gpio input events  
def remove_all_gpio_events():
  global gpio_callbacks
  for stack in gpio_callbacks:
    for ch in gpio_callbacks[stack]:
      remove_gpio_event(stack, ch, RISING)
      remove_gpio_event(stack, ch, FALLING)
  del gpio_callbacks
  
    
# Process the interrupt
def process_isr(channel):
  bus = smbus.SMBus(1)
  for stack in opto_callbacks:
    opto_it_flags = bus.read_byte_data(DEVICE_ADDRESS + stack, OPTO_IT_FLAGS_MEM_ADD)
    opto_val = bus.read_byte_data(DEVICE_ADDRESS + stack, OPTO_IN_MEM_ADD)
    for ch in opto_callbacks[stack]:
      if ((opto_it_flags >>(ch -1))  & 1)  == 1: 
        if ((opto_val >>(ch -1)) & 1) == 1: 
          if RISING in opto_callbacks[stack][ch]:#rising edge
            opto_callbacks[stack][ch][RISING](ch)
        else:
          if FALLING in opto_callbacks[stack][ch]:
            opto_callbacks[stack][ch][FALLING](ch)
            
  for stack in gpio_callbacks: 
    gpio_it_flags = bus.read_byte_data(DEVICE_ADDRESS + stack, GPIO_IT_FLAGS_MEM_ADD)
    gpio_val = bus.read_byte_data(DEVICE_ADDRESS + stack, GPIO_VAL_MEM_ADD)
    for ch in gpio_callbacks[stack]:
      if ((gpio_it_flags >> (ch - 1)) & 1)  == 1: 
        if ((gpio_val >> (ch - 1)) & 1) == 1: 
          if RISING in gpio_callbacks[stack][ch] :#rising edge
            gpio_callbacks[stack][ch][RISING](ch)
        else:
          if FALLING in gpio_callbacks[stack][ch]:
            gpio_callbacks[stack][ch][FALLING](ch)
      
GPIO.add_event_detect(4, GPIO.FALLING, callback=process_isr, bouncetime=200) 
       