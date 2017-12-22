import smbus

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