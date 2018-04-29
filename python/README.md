
[![megaio-rpi](res/sequent.jpg)](https://www.sequentmicrosystems.com/megaio.html)

# megaio.py

This is the python library to control [Raspberry Pi Mega-IO Expansion Card](https://www.sequentmicrosystems.com/megaio.html)

Before use you have to install python smbus library

## Functions
### set_relay(stack, relay, value)
Set one relay state

stack - is the stack level of megaio card ( it can be selected from address jumpers) [0..3]
relay - relay number (id) [1..8]
value - relay state 0: turn ON, 1: turn OFF

### set_relays(stack, value)
Set all relays state

stack - is the stack level of megaio card ( it can be selected from address jumpers) [0..3]
value - is the 8 bit value of all relays (ex: 255 turn off all relays, 0 turn off all relay, 1 - turn on relay 1 and off the rest)

