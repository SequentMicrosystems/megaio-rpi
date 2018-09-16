
[![megaio-rpi](readmeres/sequent.jpg)](https://www.sequentmicrosystems.com/megaio.html)

# megaio-rpi

This is the command to control [Raspberry Pi Mega-IO Expansion Card](https://www.sequentmicrosystems.com/megaio.html)

Before compiling you have to install [Wiring Pi](http://wiringpi.com/download-and-install/), many thanks to Gordon Henderson for the library

## Usage

```bash
~$ git clone https://github.com/SequentMicrosystems/megaio-rpi.git
~$ cd megaio-rpi/
~/megaio-rpi$ sudo make install
```

Now you can access all the functions of the Mega-IO card through the "megaio" command.
If you clone the repository, any update can be made with the following commands:

```bash
~$ cd megaio-rpi/  
~/megaio-rpi$ git pull
~/megaio-rpi$ sudo make install
```  
