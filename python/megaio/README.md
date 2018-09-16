# megaio

This is the python library to control [Raspberry Pi Mega-IO Expansion Card](https://www.sequentmicrosystems.com/megaio.html).
Below you find the functions prototypes.

## Functions
### set_relay(stack, relay, value)
Set one relay state.

stack - stack level of the megaio card (selectable from address jumpers [0..3])

relay - relay number (id) [1..8]

value - relay state 1: turn ON, 0: turn OFF[0..1]

### set_relays(stack, value)
Set all relays state.

stack - stack level of the megaio card (selectable from address jumpers [0..3])

value - 8 bit value of all relays (ex: 255: turn on all relays, 0: turn off all relays, 1:turn on relay #1 and off the rest)

### get_relays(stack)
Return the state of all relays.

stack - stack level of the megaio card (selectable from address jumpers [0..3])

return - [0..255]

### get_adc(stack, channel)
Return the raw value of ADC conversion on specific channel

stack - stack level of the megaio card (selectable from address jumpers [0..3])

channel - analog input channel number [1..8]

return raw value (0 for 0V, 4095 for 3.3V)

### get_adc_volt(stack, channel)
Return the ADC measured voltage on a specific channel

stack - stack level of the megaio card (selectable from address jumpers [0..3])

channel - analog input channel number [1..8]

return voltage [0..3.3]

### set_dac(stack, value)
Set a voltage on the analog output channel (raw version)

stack - stack level of the megaio card (selectable from address jumpers [0..3])

value - raw value (0 for 0V, 4095 for 3.3V) [0...4095]

### set_dac_volt(stack, value)
Set a voltage on the analog out channel

stack - stack level of the megaio card (selectable from address jumpers [0..3])

value - value in volts [0..3.3]

### get_dac(stack)
Get the raw value previously set to the DAC.

stack - stack level of the megaio card (selectable from address jumpers [0..3])

return - raw value (0 for 0V, 4095 for 3.3V) [0...4095]

### get_opto_in(stack)
Get the state of the opto-coupled inputs

stack - stack level of the megaio card (selectable from address jumpers [0..3])

return - value of the inputs [0..255]

### set_io_pin_dir(stack, pin, dir)
Set the corresponding digital I/O pin direction

stack - stack level of the megaio card (selectable from address jumpers [0..3])

pin - I/O pin number [1..6]

dir - I/O pin direction 0: output; 1: input[0..1]

### get_io_val(stack)
Get the state of digital I/O pins which are set as outputs

stack - stack level of the megaio card (selectable from address jumpers [0..3])

return - the state of I/O pins (ex 0 - all pins are LOW; 63 - all pins are HIGH; 1 - pin 1 HIGH rest LOW)

### set_io_pin(stack, pin, val)
Set the state of corresponding I/O pins set as outputs; ignore pins set as inputs

stack - stack level of the megaio card (selectable from address jumpers [0..3])

pin - I/O pin number [1..6]

val - state of I/O pin 0: LOW, 1: HIGH [0..1]

### set_oc_pin(stack, pin, val)
Set the state of the corresponding open-collector output pin

stack - stack level of the megaio card (selectable from address jumpers [0..3])

pin - open-collector pin # [1..4]

val - state 0: ON, 1: OFF [0..1]

### get_oc_val(stack)
Get the previously set state of all open-collector output pins

stack - stack level of the megaio card (selectable from address jumpers [0..3])

return - state of pins ( ex: 0: all pins OFF, 15: all pins ON, 1:  pin #1 ON, all the rest: OFF)

### add_opto_event(stack, ch, edge, callback)
Add a function to be called when a change of a optocupled input pin status occur ( also enable the interrupt on selected edge and channel);

stack - stack level of the megaio card (selectable from address jumpers [0..3])

ch - optocupled channel number [1..8]

edge - RISING or FALLING

callback - the name of the function to be called.

### remove_opto_event(stack, ch, edge)
Remove previus added callback and disable interrupt on selected channel and edge.

stack - stack level of the megaio card (selectable from address jumpers [0..3])

ch - optocupled channel number [1..8]

edge - RISING or FALLING

### remove_all_opto_events()
Remove all callbacks and disable all interrupts on every megaio card connected

### add_gpio_event(stack, ch, edge, callback)
Add a function to be called when a change of a gpio input pin status occur ( also enable the interrupt on selected edge and channel);

stack - stack level of the megaio card (selectable from address jumpers [0..3])

ch - gpio channel number [1..6]

edge - RISING or FALLING

callback - the name of the function to be called.

### remove_gpio_event(stack, ch, edge)
Remove previus added callback and disable interrupt on selected channel and edge.

stack - stack level of the megaio card (selectable from address jumpers [0..3])

ch - gpio channel number [1..6]

edge - RISING or FALLING

### remove_all_gpio_events()
Remove all callbacks and disable all gpio interrupts on every megaio card connected
