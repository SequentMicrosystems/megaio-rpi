
[![megaio-rpi](res/sequent.jpg)](https://www.sequentmicrosystems.com/megaio.html)

# megaio

This is the python library to control [Raspberry Pi Mega-IO Expansion Card](https://www.sequentmicrosystems.com/megaio.html)

## Install

```bash
~$ sudo apt-get update
~$ sudo apt-get install build-essential python-pip python-dev python-smbus git
~$ git clone https://github.com/SequentMicrosystems/megaio-rpi.git
~$ cd megaio-rpi/python/megaio/
~/megaio-rpi/python/megaio$ sudo python setup.py install
```
## Update

```bash
~$ cd megaio-rpi/
~/megaio-rpi$ git pull
~$ cd megaio-rpi/python/megaio/
~/megaio-rpi/python/megaio$ sudo python setup.py install
```

## Usage 

Before using the library make sure you disable 1-wire interface (raspi-config), because the board use the pin7 (GPIO4 / 1-wire bus pin) for interrupt.
Now you can import the megaio library and use its functions. To test, read relays status from the MegaIO board with stack level 0:

```bash
~$ python
Python 2.7.9 (default, Sep 17 2016, 20:26:04)
[GCC 4.9.2] on linux2
Type "help", "copyright", "credits" or "license" for more information.
>>> import megaio
>>> megaio.get_relays(0)
0
>>>
```
Prototypes for all functions can be found in README.md file locate under megaio-rpi/python/megaio/ directory. 

This library works with both Python2.x and Python3

For interrupt driven usage please check int_example.py file
