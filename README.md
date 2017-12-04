# megaio-rpi
--------------------------------------------
This is the command to control Raspberry Pi Mega-IO Expansion Card 
https://www.sequentmicrosystems.com/megaio.html

Before compiling you have to install Wiring Pi 
http://wiringpi.com/download-and-install/ (many thanks to Gordon Henderson for the library)

## Usage
--------------------------------------------
After you clone or download the source code, run the following commands:

  ~$ cd megaio-rpi/
  
  ~/megaio-rpi$ sudo make install
  
 Now you can access all the functions of the Mega-IO board through the command "megaio"
 If you clone the repository any update can be made with the following commands:
 
  ~$ cd megaio-rpi/
  
  ~/megaio-rpi$ git pull

  ~/megaio-rpi$ sudo make install
  
