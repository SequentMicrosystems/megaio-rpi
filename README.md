
[![megaio-rpi](readmeres/sequent.jpg)](https://sequentmicrosystems.com)

# megaio-rpi

This is the command to control Old Raspberry Pi [Mega-IO](https://sequentmicrosystems.com/docs/MEGA-IO-UsersGuide.pdf) Expansion Card

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

For python library install and usage checkout the "python" subfolder.
