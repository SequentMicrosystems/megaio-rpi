
[![megaio-rpi](res/sequent.jpg)](https://www.sequentmicrosystems.com/megaio.html)

# megaio

This is the python library to control [Raspberry Pi Mega-IO Expansion Card](https://www.sequentmicrosystems.com/megaio.html)

Before use you have to install python smbus library

## Install

```bash
~$ sudo apt-get update
~$ sudo apt-get install build-essential python-pip python-dev python-smbus git
~$ git clone https://github.com/SequentMicrosystems/megaio-rpi.git
~$ cd megaio-rpi/python/megaio/
~/megaio-rpi/python/megaio$ sudo python setup.py install
```

## Usage 

Now you can import megaio library and use is functions, for test we read relays status from MegaIO board with stack level 0:

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
All functions prototypes can be found in README.md file locate under megaio-rpi/python/megaio/ directory 
You can install and use the library under python 3 if you like.

