
[![megaio-rpi](res/sequent.jpg)](https://www.sequentmicrosystems.com/megaio.html)

# megaio.py

This is the python library to control [Raspberry Pi Mega-IO Expansion Card](https://www.sequentmicrosystems.com/megaio.html)

Before use you have to install python smbus library

## Functions
### set_relay(stack, relay, value)
Set one relay state.

stack - is the stack level of megaio card ( it can be selected from address jumpers) [0..3]

relay - relay number (id) [1..8]

value - relay state 1: turn ON, 0: turn OFF[0..1]

### set_relays(stack, value)
Set all relays state.

stack - is the stack level of megaio card ( it can be selected from address jumpers) [0..3]

value - is the 8 bit value of all relays (ex: 255 turn on all relays, 0 turn off all relay, 1 - turn on relay no.1 and off the rest)

### get_relays(stack)
Return the state of all relays.

stack - is the stack level of megaio card ( it can be selected from address jumpers) [0..3]

return - [0..255]

### get_adc(stack, channel)
Return the raw value of ADC conversion on specific channel

stack - is the stack level of megaio card ( it can be selected from address jumpers) [0..3]

channel - analog in channel number [1..8]

return  raw value (0 for 0V, 4095 for 3.3V)

### get_adc_volt(stack, channel)
Return the ADC measured voltage on specific channel

stack - is the stack level of megaio card ( it can be selected from address jumpers) [0..3]

channel - analog in channel number [1..8]

return  voltage [0..3.3]

### set_dac(stack, value)
Set a voltage on the analog out channel (is only one)(raw version)

stack - is the stack level of megaio card ( it can be selected from address jumpers) [0..3]

value - raw value (0 for 0V, 4095 for 3.3V) [0...4095]

### set_dac_volt(stack, value)
Set a voltage on the analog out channel (is only one)

stack - is the stack level of megaio card ( it can be selected from address jumpers) [0..3]

value - value in volts [0..3.3]

### get_dac(stack)
Get the previously set raw value to the DAC.

stack - is the stack level of megaio card ( it can be selected from address jumpers) [0..3]

return - raw value (0 for 0V, 4095 for 3.3V) [0...4095]

### get_opto_in(stack)
Get the state of the optocouppled inputs

stack - is the stack level of megaio card ( it can be selected from address jumpers) [0..3]

return - value of the inputs [0..255]

### set_io_pin_dir(stack, pin, dir)
Set the coresponded digital I/O pin direction

stack - is the stack level of megaio card ( it can be selected from address jumpers) [0..3]

pin - I/O pin number [1..6]

dir - I/O pin direcion 0: output; 1: input[0..1]

### get_io_val(stack)
Get the state of digital I/O pins, for the pin that are set as output it read the output stat 

stack - is the stack level of megaio card ( it can be selected from address jumpers) [0..3]
 
return - the state of I/O pins (ex 0 - all pins are LOW; 63 - all pins are HIGH; 1 - pin 1 HIGH rest LOW)

### set_io_pin(stack, pin, val)
Set the state of coresponding I/O pin if is set as output, else is ignored

stack - is the stack level of megaio card ( it can be selected from address jumpers) [0..3]

pin - I/O pin number [1..6]

val - state of I/O pin 0 - LOW, 1 - HIGH [0..1]

### set_oc_pin(stack, pin, val)
Set the coresponded open-collector output pin state

stack - is the stack level of megaio card ( it can be selected from address jumpers) [0..3]

pin - open-collector pin number [1..4]

val - state 0: ON, 1: OFF [0..1]

### get_oc_val(stack)
Get the previously set state of all open-collector output pins

stack - is the stack level of megaio card ( it can be selected from address jumpers) [0..3]

return - state of pins ( ex: 0 - all pins OFF, 15 - all pins ON, 1 - pin 1 ON the rest of all OFF)

