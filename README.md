
[![megaio-rpi](readmeres/sequent.jpg)](https://sequentmicrosystems.com)

# megaio-rpi

This is the command to control Old Raspberry Pi [Mega-IO](https://sequentmicrosystems.com/docs/MEGA-IO-UsersGuide.pdf) Expansion Card

Before compiling you have to install [Wiring Pi](http://wiringpi.com/download-and-install/), many thanks to Gordon Henderson for the library.

Since the WiringPi are not longer maintained by his creator, to install it, get the latest .deb version from:

https://github.com/WiringPi/WiringPi/releases/ 

The -armhf.deb file should be used on 32-bit OS (Raspberry Pi 3 and under) and the -arm64.deb is meant for 64-bit OS (Raspberry Pi 4 and up). Download the appropriate file for your architecture on your Raspberry Pi and install it with the dpkg command:
```bash
dpkg -i wiringpi-[version]-armhf.deb

or

dpkg -i wiringpi-[version]-arm64.deb
```

Test that the WiringPi installation finished successfully with the command:
```bash
gpio -v
```

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
