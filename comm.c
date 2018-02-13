/*
 * comm.c:
 *	Communication routines "platform specific" for Raspberry Pi
 *	
 *	Copyright (c) 2016-2018 Sequent Microsystem
 *	<http://www.sequentmicrosystem.com>
 ***********************************************************************
 *	Author: Alexandru Burcea
 ***********************************************************************
 */
#include <stdio.h>
#include <termios.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <math.h>
#include "megaio.h"

static volatile int globalResponse = 0;
int gLed1HwAdd = 0x20;
int gLed2HwAdd = 0x21;

PI_THREAD (waitForKey)
{
 char resp;
 int respI = NO;

 
	struct termios info;
	tcgetattr(0, &info);          /* get current terminal attirbutes; 0 is the file descriptor for stdin */
	info.c_lflag &= ~ICANON;      /* disable canonical mode */
	info.c_cc[VMIN] = 1;          /* wait until at least one keystroke available */
	info.c_cc[VTIME] = 0;         /* no timeout */
	tcsetattr(0, TCSANOW, &info); /* set i */

	(void)piHiPri (10) ;	// Set this thread to be high priority
	resp = getchar();
	if((resp == 'y') || (resp == 'Y'))
		respI = YES;
	
    piLock (COUNT_KEY) ;
	globalResponse = respI ;
    piUnlock (COUNT_KEY) ;
	
	info.c_lflag |= ICANON;      /* disable canonical mode */
	info.c_cc[VMIN] = 0;          /* wait until at least one keystroke available */
	info.c_cc[VTIME] = 0;         /* no timeout */
	tcsetattr(0, TCSANOW, &info); /* set i */
	printf("\n");
	return &waitForKey;
}

void startThread(void)
{
	wiringPiSetupSys ();
	piThreadCreate (waitForKey);
}

int checkThreadResult(void)
{
	int res;
	piLock (COUNT_KEY) ;
	res = globalResponse;
	piUnlock(COUNT_KEY);
	return res;
}
	
int readReg16(int dev, int add)
{
	int val, ret;
	
	val = wiringPiI2CReadReg16(dev, add);
	ret = 0xff & (val >> 8);
	ret+= 0xff00 & (val << 8);
	return ret;
}

int writeReg16(int dev, int add, int val)
{
	int wVal;
	
	wVal = 0xff & (val >> 8);
	wVal += 0xff00 & (val << 8);
	wiringPiI2CWriteReg16(dev,add, wVal);
	delay(1);
	return 0;
}


int writeReg8(int dev, int add, int val)
{
	wiringPiI2CWriteReg8(dev, add, val);
	
	return 0;
}

int readReg8(int dev, int add)
{
	return wiringPiI2CReadReg8(dev, add);
}


int readReg24(int dev, int add)
{
	int val, aux8;
	
	aux8 = readReg8(dev, add + 2);
	val = aux8;
	/*val = 0xffff00 & (val << 8);*/
	aux8 = readReg8(dev, add + 1);
	val += 0xff00 & (aux8 << 8);
	
	aux8 = readReg8(dev, add );
	val += 0xff0000 & (aux8 << 16);
#ifdef DEBUG_I	
	printbits(val);
	printf("\n");
	printf("%#08x\n", val);
#endif
	return val;
}



int writeReg24(int dev, int add, int val)
{
	int wVal;//, aux8;
	
	wVal = 0xff & (val >> 8);
	writeReg8(dev,add+1, wVal);
	
	wVal = 0xff & ( val >> 16);
	writeReg8(dev,add, wVal);
	
	wVal = 0xff & val;
	writeReg8(dev,add+2, wVal);
	
	return 0;
}

	
int doBoardInit(int hwAdd)
{
	int dev, bV = -1;
	dev = wiringPiI2CSetup (hwAdd);
	if(dev == -1)
	{
		return ERROR;
	}
	bV = wiringPiI2CReadReg8 (dev,REVISION_HW_MAJOR_MEM_ADD);
	if(bV == -1)
	{
		printf( "MegaIO id %d not detected\n", hwAdd - MEGAIO_HW_I2C_BASE_ADD);
		return ERROR;
	}
	return dev;
}


/*
* getLedVal
* Get the value of leds 
* arg: chip 0 - 1
* ret: 0x0000 - 0xffff - success; -1 - fail 
*/
int getLedVal(int chip)
{
	int dev = -1;
	int ret = 0;
	u16 rVal = 0;
	
	if((chip < 0) || (chip > 1))
	{
		return -1;
	}
	dev = wiringPiI2CSetup(gLed1HwAdd + chip); 
	if(dev <= 0)
	{
		return -1;
	}
	ret  = wiringPiI2CReadReg16(dev, 0x02);
	if(ret < 0)
	{
		return -1;
	}
	rVal = 0xff00 & (ret << 4);
	rVal += 0x01 & (ret >> 3);
	rVal += 0x02 & (ret >> 1);
	rVal += 0x04 & (ret << 1);
	rVal += 0x08 & (ret << 3);
	rVal += 0x10 & (ret >> 11);
	rVal += 0x20 & (ret >> 9);
	rVal += 0x40 & (ret >> 7);
	rVal += 0x80 & (ret >> 5);
	
	return rVal;
}
	

/*
* setLedVal
* Get the value of leds 
* arg: chip 0 - 1
* arg: val 0x0000 - 0xffff 
* ret: 0 - success; -1 - fail 
*/
int setLedVal(int chip, int val)
{
	int dev = -1;
	int ret = 0;
	u16 wVal = 0;
	
	if((chip < 0) || (chip > 1))
	{
		return -1;
	}
	if((val < 0) || (val > 0xffff))
	{
		return -1;
	}
	
	dev = wiringPiI2CSetup(gLed1HwAdd + chip); 
	if(dev <= 0)
	{
		return -1;
	}
	wVal = 0x0ff0 & ( val >> 4);
	wVal += 0x1000 & (val << 5);
	wVal += 0x2000 & (val << 7);
	wVal += 0x4000 & (val << 9);
	wVal += 0x8000 & (val << 11);
	wVal += 0x0001 & (val >> 3);
	wVal += 0x0002 & (val >> 1);
	wVal += 0x0004 & (val << 1);
	wVal += 0x0008 & (val << 3);
	
	//wVal = 0xffff & val;
	wiringPiI2CWriteReg16(dev, 0x06, 0x0000); // set all to output
	wiringPiI2CWriteReg16(dev, 0x02, wVal);
	
	return ret;
}	
	
/*
* wrLeds
* Write leds as a corresponence of relay 
*/
#ifdef LED_RELAY_LINK
static int wrLedsR(int stack, int val)
{
	int dev, addOut = 0x02;
	if(stack < 2)
	{
		dev = wiringPiI2CSetup(gLed1HwAdd);
	}
	else if(stack < 4)
	{
		dev = wiringPiI2CSetup(gLed2HwAdd);
	}
	else
	{
#ifdef DEBUG_LED
		printf("Invalid stack level\n");
#endif
		return -1;
	}
	if(val < 0 || val > 255)
	{
#ifdef DEBUG_LED
		printf("Led value out of range\n");
#endif
		return -1;
	}
	if(dev == -1)
	{
#ifdef DEBUG_LED
		printf("No led board detected\n");
#endif
		return -1;
	}
	
	addOut += (stack & 0x01); 
	
	wiringPiI2CWriteReg16(dev, 0x06, 0x0000);
	
	wiringPiI2CWriteReg8(dev, addOut, 0xff & (~val));
// todo: check write succesful
	return 1;
}
#endif
void busyWait(int ms)
{
	delay(ms);
}
	