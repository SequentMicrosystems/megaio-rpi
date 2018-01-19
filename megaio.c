/*
 * megaio.c:
 *	Command-line interface to the Raspberry
 *	Pi's MegaIO board.
 *	Copyright (c) 2016-2017 Sequent Microsystem
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

#define	COUNT_KEY	0
#define YES		1
#define NO		2
#define ADC_TEST_VAL_LOW	2850 	
#define ADC_TEST_VAL_HIGH	3300	

#define DEBUG_LED
//#define DEBUG_TEST 1
//#define TEST_RESET 1

int gHwAdd = 0x31;
int gLed1HwAdd = 0x20;
int gLed2HwAdd = 0x21;


char *usage = "Usage:	megaio -h <command>\n"
		"         megaio -v\n"
		"         megaio -lt\n"
		"         megaio -lw <val>\n"
		"         megaio -lw <lednr> <val>\n"
		"         megaio -warranty\n"
		"         megaio <id> board\n"
	      	"         megaio <id> rwrite <channel> <on/off>\n"
              	"         megaio <id> rread <channel>\n" 
              	"         megaio <id> rread\n"	
              	"         megaio <id> aread <channel>\n"
              	"         megaio <id> awrite <value>\n"
              	"         megaio <id> optread <channel>\n"
              	"         megaio <id> optread\n"
				"         megaio <id> ocread\n"
				"         megaio <id> ocwrite <ch> <on/off; 1/0>\n"
				"         megaio <id> ocwrite <val>\n"
              	"         megaio <id> optirqset <channel> <rising/falling/change/none>\n"
			"         megaio <id> optitRead\n"
              	"         megaio <id> iodwrite <channel> <in/out>\n"
              	"         megaio <id> iodread <channel>\n"
              	"         megaio <id> iodread\n" 
              	"         megaio <id> iowrite <channel> <on/off>\n"
              	"         megaio <id> ioread <channel>\n"
              	"         megaio <id> ioread\n"
              	"         megaio <id> ioirqset <channel> <rising/falling/change/none>\n"
		"         megaio <id> ioitread\n"
		"         megaio <id> test\n"
		"         megaio <id> atest <chNr>\n"
		"Where: <id> = Board level id = 0..4\n"
		"Type megaio -h <command> for more help";// No trailing newline needed here.
			  
char *warranty ="	       Copyright (c) 2016-2017 Sequent Microsystems\n"
				"                                                             \n"
				"		This program is free software; you can redistribute it and/or modify\n"
				"		it under the terms of the GNU Leser General Public License as published\n"
				"		by the Free Software Foundation, either version 3 of the License, or\n"
				"		(at your option) any later version.\n"
				"                                    \n"
				"		This program is distributed in the hope that it will be useful,\n"
				"		but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
				"		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
				"		GNU Lesser General Public License for more details.\n"
				"			\n"
				"		You should have received a copy of the GNU Lesser General Public License\n"
				"		along with this program. If not, see <http://www.gnu.org/licenses/>.";
		  
static volatile int globalResponse = 0 ;

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
	if(resp == 'y')
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

int readReg16(int dev, int add)
{
	int val, ret;
	
	val = wiringPiI2CReadReg16(dev, add);
	ret = 0xff & (val >> 8);
	ret+= 0xff00 & (val << 8);
	//delay(1); //megaio <id> optirq <channel> <rising/falling/change/none>\n"

	return ret;
}

void printbits(int v) 
{
	int i; // for C89 compatability
	
	for(i = 17; i >= 0; i--) putchar('0' + ((v >> i) & 1));
}

int readReg24(int dev, int add)
{
	int val, aux8;
	
	aux8 = wiringPiI2CReadReg8(dev, add + 2);
	val = aux8;
	/*val = 0xffff00 & (val << 8);*/
	aux8 = wiringPiI2CReadReg8(dev, add + 1);
	val += 0xff00 & (aux8 << 8);
	
	aux8 = wiringPiI2CReadReg8(dev, add );
	val += 0xff0000 & (aux8 << 16);
#ifdef DEBUG_I	
	printbits(val);
	printf("\n");
	printf("%#08x\n", val);
#endif
	return val;
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

int writeReg24(int dev, int add, int val)
{
	int wVal;//, aux8;
	
	
	//aux8 = wiringPiI2CReadReg8(dev, add );
	wVal = 0xff & (val >> 8);
	wiringPiI2CWriteReg8(dev,add+1, wVal);
	
	//aux8 = wiringPiI2CReadReg8(dev, add );
	wVal = 0xff & ( val >> 16);
	wiringPiI2CWriteReg8(dev,add, wVal);
	
	//aux8 = wiringPiI2CReadReg8(dev, add );
	wVal = 0xff & val;
	wiringPiI2CWriteReg8(dev,add+2, wVal);
	
	return 0;
}

static int doBoardInit(int hwAdd)
{
	int dev, bV = -1;
	dev = wiringPiI2CSetup (hwAdd);
	if(dev == -1)
	{
		return -1;
	}
	bV = wiringPiI2CReadReg8 (dev,REVISION_HW_MAJOR_MEM_ADD);
	if(bV == -1)
	{
		return -1;
	}
	return dev;
}

/*
* getLedVal
* Get the value of leds 
* arg: chip 0 - 1
* ret: 0x0000 - 0xffff - success; -1 - fail 
*/
static int getLedVal(int chip)
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
static int setLedVal(int chip, int val)
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
	
static int doLed(int argc, char *argv[])
{
	unsigned int val;
	u16 setVal, getVal;
	int ledNr;
	int ledVal;
	int ret = 0;
	int ledChip = 0;
	
	if(argc == 3) // call by value
	{
		ret = sscanf(argv[2], "%x", &val);
		if(ret <= 0)
		{
			printf("Invalid LED value\n");
			exit(1);
		}
		val = ~val; // active low
		ret = 0;
		setVal = 0xffff & val;
		ret += setLedVal(1, setVal);
		setVal = 0xffff & (val >> 16);
		ret += setLedVal(0, setVal);
		if(ret < 0)
		{
			printf("Fail to write leds\n");
			return -1;
		}
		else
		{
			return 0;
		}
	}
	else if(argc == 4)
	{
		ledNr = atoi(argv[2]);
		if((ledNr < 0) || (ledNr > 32))
		{
			printf("Invalid LED number\n");
			return -1;
		}
		ledVal = -1;
		if((strcasecmp(argv[3], "on") == 0) || (strcasecmp(argv[3], "high") == 0)|| (strcasecmp(argv[3], "1") == 0))
		{
			ledVal = 0;
		}
		if((strcasecmp(argv[3], "off") == 0) || (strcasecmp(argv[3], "low") == 0)|| (strcasecmp(argv[3], "0") == 0))
		{
			ledVal = 1;
		}
		if(ledVal == -1)
		{
			printf("Invalid LED value\n");
			return -1;
		}
		if(ledNr < 17)
		{
			ledChip = 1;
		}
		else
		{
			ledChip = 0;
			ledNr -= 16;
		}
		ret = getLedVal(ledChip);
		if(ret < 0)
		{
			printf("Fail to comm with LED's\n");
			return -1;
		}
		getVal = 0xffff & ret;
		setVal = (u16)1 << (ledNr -1);
		if(ledVal == 1)
		{
			setVal = getVal | setVal;
		}
		else
		{
			setVal = getVal & (~setVal);
		}
		ret = setLedVal(ledChip, setVal);
		if( ret < 0)
		{
			printf("Fail to write LED's\n");
			return -1;
		}
	}
	else
	{
		printf("Invalid parameter number\n");		
	}
	return -1;
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
/*
* doLedTest
* Testing the led board
*/
static void doLedTest(void)
{
	int led, val;
	
	val = 0;
	for(led = 0; led < 32; led++)
	{
		val += 1 << led;
		setLedVal(1, 0xffff & ~val);
		setLedVal(0, 0xffff & ~(val >> 16));
		delay(100);
	}
	delay(250);
	val = 0;
	for(led = 0; led < 32; led++)
	{
		val += 1 << led;
		setLedVal(1, 0xffff & val);
		setLedVal(0, 0xffff & (val >> 16));
		delay(100);
	}
}

/*
* doRelayWrite:
*	Write coresponding relay channel
**************************************************************************************
*/
static void doRelayWrite(int argc, char *argv[])
{
	int pin, val, dev, valR, valRmask, retry;

	if ((argc != 5) && (argc != 4))
	{
		printf( "Usage: megaio <id> rwrite <relay number> <on/off> \n") ;
		printf( "Usage: megaio <id> rwrite <relay reg value> \n") ;
		exit (1) ;
	}

	dev = doBoardInit (gHwAdd);
	if(dev <= 0)
	{
		printf( "MegaIO id %d not detected\n", gHwAdd-0x31);
		exit(1);
	}
	if(argc == 5)
	{
		pin = atoi (argv [3]) ;
		if((pin < 1) || (pin > 8))
		{
			printf( "Relay number value out of range\n");
			exit(1);
		}

		/**/ if ((strcasecmp (argv [4], "up") == 0) || (strcasecmp (argv [4], "on") == 0))
		val = 1 ;
		else if ((strcasecmp (argv [4], "down") == 0) || (strcasecmp (argv [4], "off") == 0))
			val = 0 ;
		else
		{
			val = atoi (argv [4]) ;
		}
		if (val == 0)
		{
			wiringPiI2CWriteReg8 (dev,RELAY_OFF_MEM_ADD, pin);
		}
		else
		{
			wiringPiI2CWriteReg8 (dev,RELAY_ON_MEM_ADD, pin);
		}
		valR = wiringPiI2CReadReg8(dev, RELAY_MEM_ADD);
		valRmask = 0x01 & (valR >> (pin - 1));
		retry = RETRY_TIMES;

		while((retry > 0) && (valRmask != val))
		{
			if (val == 0)
			{
				wiringPiI2CWriteReg8 (dev,RELAY_OFF_MEM_ADD, pin);
			}
			else
			{
				wiringPiI2CWriteReg8 (dev,RELAY_ON_MEM_ADD, pin);
			}
			valR = wiringPiI2CReadReg8(dev, RELAY_MEM_ADD);
			valRmask = 0x01 & (valR >> (pin - 1));
			retry--;
		}
		#ifdef DEBUG_I
		if(retry < RETRY_TIMES)
		{
			printf("retry %d times\n", 3-retry);
		}
		#endif
		if(retry == 0)
		{
			printf( "Fail to write relay\n");
			exit(1);
		}
	}
	else
	{
		val = atoi (argv [3]) ;
		if(val < 0 || val > 255)
		{
			printf( "Invalid relay value\n");
			exit(1);
		}
		retry = RETRY_TIMES;
		while((retry > 0) && (valR != val))
		{
			wiringPiI2CWriteReg8 (dev,RELAY_MEM_ADD, val);
			valR = wiringPiI2CReadReg8(dev, RELAY_MEM_ADD);
		}
		if(retry == 0)
		{
			printf( "Fail to write relay\n");
			exit(1);
		}
	}
#ifdef LED_RELAY_LINK
	wrLedsR(gHwAdd-0x31, valR);
#endif
}


/*
* doRelayRead:
*	Read relay state
******************************************************************************************
*/
static void doRelayRead(int argc, char *argv[])
{
	int pin, val, dev;

	dev = doBoardInit (gHwAdd);
	if(dev <= 0)
	{
		printf( "MegaIO id %d not detected\n", gHwAdd-0x31);
		exit(1);
	}

	if (argc == 4)
	{
		pin = atoi (argv [3]) ;
		if((pin < 1) || (pin > 8))
		{
			printf( "Relay number value out of range\n");
			exit(1);
		}

		val = wiringPiI2CReadReg8(dev, RELAY_MEM_ADD);
		val = val & (1 << (pin-1));
		if(val != 0)
		{
			printf("1\n");
		}
		else
		{
			printf("0\n");
		}
	}
	else if(argc == 3)
	{
		val = wiringPiI2CReadReg8(dev, RELAY_MEM_ADD);
		printf("%d\n", val);
	}
	else
	{
		printf( "Usage: %s read relay value\n", argv [0]) ;
		exit (1) ;
	}
}


/*
* doAnalogRead:
* Read analog channel
***********************************************************************************
*/
static void doAnalogRead(int argc, char *argv[])
{
	int pin, dev, addr, rd;

	dev = doBoardInit (gHwAdd);
	if(dev <= 0)
	{
		printf( "MegaIO id %d not detected\n", gHwAdd-0x31);
		exit(1);
	}

	if (argc == 4)
	{
		pin = atoi (argv [3]) ;
		if((pin < 1) || (pin > 8))
		{
			printf( "Analog channel number %d out of range\n", pin);
			exit(1);
		}

		addr = ADC_VAL_MEM_ADD + 2* (pin-1);
		rd = readReg16(dev, addr);
		printf("%d\n", rd);
	}
	else 
	{
		printf( "Usage: %s analog read \n", argv [0]) ;
		exit (1) ;
	} 
}


/*
* doAnalogWrite:
*    Write the DAC value
***********************************************************************************************
*/

static void doAnalogWrite(int argc, char *argv[])
{
	int dev, val;
	
	dev = doBoardInit (gHwAdd);
	if(dev <= 0)
	{
		printf( "MegaIO id %d not detected\n", gHwAdd-0x31);
		exit(1);
	}
	if (argc != 4)
	{
		printf( "Usage: %s analog write\n", argv [0]) ;
		exit (1) ;
	}
	val = atoi(argv[3]);
	if((val < 0) || (val > 4095))
	{
		printf( "analog write value out of range\n");
		exit(1);
	}
	writeReg16(dev, DAC_VAL_H_MEM_ADD, val);
}


/*
* doOptoInRead:
*	Read the optocupled input pins
************************************************************************************************
*/

static void doOptoInRead(int argc, char *argv[])
{
  int pin, val, dev;

	dev = doBoardInit(gHwAdd);
	if(dev <= 0)
	{
		printf( "MegaIO id %d not detected\n", gHwAdd-0x31);
		exit(1);
	}
  
	if (argc == 4)
	{
		pin = atoi (argv [3]) ;
		if((pin < 1) || (pin > 8))
		{
			printf( "Opto In channel out of range\n");
			exit(1);
		}

		val = wiringPiI2CReadReg8(dev, OPTO_IN_MEM_ADD);
		val = val & (1 << (pin-1));
		if(val != 0)
		{
			printf("1\n");
		}
		else
		{
			printf("0\n");
		}
	}
	else if(argc == 3)
	{
		val = wiringPiI2CReadReg8(dev, OPTO_IN_MEM_ADD);
		printf("%d\n", val);
	}
	else
	{
		printf( "Usage: %s read opto pins \n", argv [0]) ;
		exit (1) ;
	} 
}

/*
* doOptoInIrq:
*	Set the optocupled input interrupt 
************************************************************************************************
*/

static void doOptoInIrq(int argc, char *argv[])
{
  int pin, val, rRising, rFalling;
  int dev;

  dev = doBoardInit (gHwAdd);
  if(dev <= 0)
  {
    printf( "MegaIO id %d not detected\n", gHwAdd-0x31);
    exit(1);
  }
  if (argc == 5)
	{
		pin = atoi (argv [3]) ;
		if((pin < 1) || (pin > 8))
		{
			printf( "Opto in pin number value out of range\n");
			exit(1);
		}
	  
		/**/ if ((strcasecmp (argv [4], "rising") == 0) || (strcasecmp (argv [4], "high") == 0))
		{
			val = 1 ;
		}
		else if ((strcasecmp (argv [4], "falling") == 0) || (strcasecmp (argv [4], "low") == 0))
		{
			val = 2 ;
		}
		else if ((strcasecmp (argv [4], "bowth") == 0) || (strcasecmp (argv [4], "change") == 0))
		{
			val = 3 ;
		}
		else if ((strcasecmp (argv [4], "disable") == 0) || (strcasecmp (argv [4], "none") == 0))
		{
			val = 0;
		}
		else
		{
			val = atoi (argv [4]);
		}
	
		rRising = wiringPiI2CReadReg8(dev,OPTO_IT_RISING_MEM_ADD );
		delay(10);
		rFalling = wiringPiI2CReadReg8(dev, OPTO_IT_FALLING_MEM_ADD);
		delay(10);
		switch(val)
		{
			case 0:
				rRising &= ~( 1 << (pin-1));
				rFalling &= ~( 1 << (pin-1));
			break;
			
			case 1:
				rRising |=  (1 << (pin-1));
				rFalling &= ~( 1 << (pin-1));
			break;
			
			case 2:
				rRising &= ~( 1 << (pin-1));
				rFalling |= ( 1 << (pin-1));
			break;
			
			case 3:
				rRising |= ( 1 << (pin-1));
				rFalling |= ( 1 << (pin-1));
			break;
			
			default:
				printf( "Invalid interrupt type %s\n", argv[4]);
				exit(1);
			break;
		}
		val = (0xff00 & (rFalling << 8)) + (0xff & rRising);
		
		wiringPiI2CWriteReg8(dev,OPTO_IT_RISING_MEM_ADD, rRising);
		delay(10);
		wiringPiI2CWriteReg8(dev,OPTO_IT_FALLING_MEM_ADD, rFalling);
		
	}
	else
	{
		printf( "megaio <id> optirq <channel> <rising/falling/change/none>\n") ;
		exit (1) ;
	}	
}
 
/*
* doOptoInIt
* Read the rise event by opto in pins..
******************************************************************************************
*/ 
static void doOptoInIt(int argc)
{
	int dev, val;
	
	dev = doBoardInit (gHwAdd);
	if(dev <= 0)
	{
		printf( "MegaIO id %d not detected\n", gHwAdd-0x31);
		exit(1);
	}
	if (argc == 3)
	{
		val = wiringPiI2CReadReg8(dev,OPTO_IT_FLAGS_MEM_ADD );
		printf("%d\n", val);
	}
	else
	{
  		printf( "Invalid command\n");
		exit(1);
	}
 } 
/*
 * doIoModeSet:
 *     Set the coresponding io pin direction
 * *******************************************************************************************
 */
 static void doIoModeSet(int argc, char *argv[])
 {
	int dev, pin, val, rVal; 
	
	dev = doBoardInit (gHwAdd);
	if(dev <= 0)
	{
		printf( "MegaIO id %d not detected\n", gHwAdd-0x31);
		exit(1);
	}
	if (argc == 5)
	{
		pin = atoi (argv [3]) ;
		if((pin < 1) || (pin > GPIO_PIN_NUMBER))
		{
			printf( "IO pin number value out of range\n");
			exit(1);
		}
	  
		/**/ if ((strcasecmp (argv [4], "in") == 0) || (strcasecmp (argv [4], "high") == 0))
		{
			val = 1 ;
		}
		else if ((strcasecmp (argv [4], "out") == 0) || (strcasecmp (argv [4], "low") == 0))
		{
			val = 0 ;
		}
		else
		{
			val = atoi (argv [4]);
		}
	    
		rVal = wiringPiI2CReadReg8(dev, GPIO_DIR_MEM_ADD);
	
		/**/ if (val == 0)
		{
			rVal &= ~( 1 << (pin-1));
		}
		else
		{
			rVal |= (1 << (pin - 1));
		}
		
			wiringPiI2CWriteReg8(dev,GPIO_DIR_MEM_ADD, 0xff & rVal);
	
		/*writeReg24(dev, GPIO_DIR_MEM_ADD, rVal); */
	}
	else if(argc == 4)
	{
		printf( "Not available on this version\n");
		/*
		val = atoi(argv[3]);
		if((val < 0) || (val >  0x3ffff))
		{
			printf( "Usage: %s invalid io mode\n", argv[3]);
			exit(1);
		}
		rVal = val;
		writeReg24(dev, GPIO_DIR_MEM_ADD, rVal); */
	}
	else
	{
		printf( "Usage: %s write io mode usage\n", argv [0]) ;
		exit (1) ;
	}
}


/*
 * doIoModeGet:
 *     Get the coresponding io pin direction
 * *******************************************************************************************
 */
 static void doIoModeGet(int argc, char *argv[])
 {
	int dev, pin, val, rVal; 
	
	dev = doBoardInit(gHwAdd);
	if(dev <= 0)
	{
		printf( "MegaIO id %d not detected\n", gHwAdd-0x31);
		exit(1);
	}
	rVal = 0x3f;
	
	if (argc == 4)
	{
		pin = atoi (argv [3]) ;
		if((pin < 1) || (pin > GPIO_PIN_NUMBER))
		{
			printf( "IO pin number value out of range\n");
			exit(1);
		}
	    
		rVal = wiringPiI2CReadReg8(dev, GPIO_DIR_MEM_ADD);
		val = 1 << (pin -1);
		val = val & rVal;
		/**/ if (val == 0)
		{
			printf("0\n");
		}
		else
		{
			printf("1\n");
		}
	}
	else if(argc == 3)
	{
		rVal = wiringPiI2CReadReg8(dev, GPIO_DIR_MEM_ADD);
		printf("%d\n", rVal);
	}
	else
	{
		printf( "Usage: %s read io mode usage\n", argv [0]) ;
		exit (1) ;
	}
}		
 
 
 /*
 * doIoSet:
 *     Set the coresponding io pin 
 * *******************************************************************************************
 */
 static void doIoSet(int argc, char *argv[])
 {
	int dev, pin, val; 
	
	dev = doBoardInit(gHwAdd);
	if(dev <= 0)
	{
		printf( "MegaIO id %d not detected\n", gHwAdd-0x31);
		exit(1);
	}
	
	
	if (argc == 5)
	{
		pin = atoi (argv [3]) ;
		if((pin < 1) || (pin > GPIO_PIN_NUMBER))
		{
			printf( "IO pin number value out of range\n");
			exit(1);
		}
	  
		/**/ if ((strcasecmp (argv [4], "on") == 0) || (strcasecmp (argv [4], "high") == 0))
		{
			val = 1 ;
		}
		else if ((strcasecmp (argv [4], "off") == 0) || (strcasecmp (argv [4], "low") == 0))
		{
			val = 0 ;
		}
		else
		{
			val = atoi (argv [4]);
		}
	
		/**/ if (val == 0)
		{
			wiringPiI2CWriteReg8(dev, GPIO_CLR_MEM_ADD, pin);
		}
		else
		{
			wiringPiI2CWriteReg8(dev, GPIO_SET_MEM_ADD, pin);
		}

	}
	else if(argc == 4)
	{
		printf( "Not available on this version\n");
		/*val = atoi(argv[3]);
		
		if((val < 0) || (val >  0x3ffff))
		{
			printf( "Usage: %s invalid io mode\n", argv[3]);
			exit(1);
		}
		rVal = val;
		writeReg24(dev, GPIO_VAL_MEM_ADD, rVal); */
	}
	else
	{
		printf( "Usage: %s write io mode usage\n", argv [0]) ;
		exit (1) ;
	}
	
}



/*
 * doIoGet:
 *     Get the coresponding io pin value
 * *******************************************************************************************
 */
 static void doIoGet(int argc, char *argv[])
 {
	int dev, pin, val, rVal; 
	
	dev = doBoardInit(gHwAdd);
	if(dev <= 0)
	{
		printf( "MegaIO id %d not detected\n", gHwAdd-0x31);
		exit(1);
	}
	rVal = 0x3f;
	
	if (argc == 4)
	{
		pin = atoi (argv [3]) ;
		if((pin < 1) || (pin > GPIO_PIN_NUMBER))
		{
			printf( "IO pin number value out of range\n");
			exit(1);
		}
	    
		rVal = wiringPiI2CReadReg8(dev, GPIO_VAL_MEM_ADD);
		val = 1 << (pin -1);
		val = val & rVal;
		/**/ if (val == 0)
		{
			printf("0\n");
		}
		else
		{
			printf("1\n");
		}
	}
	else if(argc == 3)
	{
		rVal = wiringPiI2CReadReg8(dev, GPIO_VAL_MEM_ADD);
		printf("%d\n", rVal);
	}
	else
	{
		printf( "Usage: %s write io mode usage\n", argv [0]) ;
		exit (1) ;
	}
}

/*
 * doIoIrqSet:
 *     Set the coresponding io pin direction
 * *******************************************************************************************
 */
 static void doIoIrqSet(int argc, char *argv[])
 {
	int dev, pin, val, rValRising, rValFalling; 
	
	dev = doBoardInit (gHwAdd);
	if(dev <= 0)
	{
		printf( "MegaIO id %d not detected\n", gHwAdd-0x31);
		exit(1);
	}
	if (argc == 5)
	{
		pin = atoi (argv [3]) ;
		if((pin < 1) || (pin > GPIO_PIN_NUMBER))
		{
			printf( "IO pin number value out of range\n");
			exit(1);
		}
	  
		/**/ if ((strcasecmp (argv [4], "rising") == 0) || (strcasecmp (argv [4], "high") == 0))
		{
			val = 1 ;
		}
		else if ((strcasecmp (argv [4], "falling") == 0) || (strcasecmp (argv [4], "low") == 0))
		{
			val = 2 ;
		}
		else if ((strcasecmp (argv [4], "bowth") == 0) || (strcasecmp (argv [4], "change") == 0))
		{
			val = 3 ;
		}
		else if ((strcasecmp (argv [4], "disable") == 0) || (strcasecmp (argv [4], "none") == 0))
		{
			val = 0;
		}
		else
		{
			val = atoi (argv [4]);
		}
	    
		rValRising = wiringPiI2CReadReg8(dev, GPIO_EXT_IT_RISING_MEM_ADD);
		delay(10);
		rValFalling = wiringPiI2CReadReg8(dev, GPIO_EXT_IT_FALLING_MEM_ADD);
		switch(val)
		{
			case 0: // disable
				rValRising &= ~( (u8)1 << (pin-1));
				rValFalling &= ~( (u8)1 << (pin-1));
			break;
			
			case 1: //rising
				rValRising |=  (u8)1 << (pin-1);
				rValFalling &= ~( (u8)1 << (pin-1));
			break;
			
			case 2: // falling
				rValFalling |=  (u8)1 << (pin-1);
				rValRising &= ~( (u8)1 << (pin-1));
			break;
			
			case 3: //both
				rValFalling |=  (u8)1 << (pin-1);
				rValRising |=  (u8)1 << (pin-1);
			break;
			 
			default:
				printf( "Invalid io irq type %s\n", argv[4] );
				exit(1);
			break;
		}
		delay(10);
		
		wiringPiI2CWriteReg8(dev,GPIO_EXT_IT_RISING_MEM_ADD, 0xff & rValRising );
		delay(10);
		wiringPiI2CWriteReg8(dev,GPIO_EXT_IT_FALLING_MEM_ADD, 0xff & rValFalling );
		
		
		
	}
	else if(argc == 4)
	{
		printf( "Not available on this version\n");
		/*
		val = atoi(argv[3]);
		if((val < 0) || (val >  0x3ffff))
		{
			printf( "Usage: %s invalid io mode\n", argv[3]);
			exit(1);
		}
		rVal = val;
		writeReg24(dev, GPIO_DIR_MEM_ADD, rVal); */
	}
	else
	{
		printf( "Usage: %s write io mode usage\n", argv [0]) ;
		exit (1) ;
	}
}


/*
* doIoIt
* Read the rise event by XPIO pins..
******************************************************************************************
*/ 
static void doIoIt(int argc)
{
	int dev, val;
	
	dev = doBoardInit (gHwAdd);
	if(dev <= 0)
	{
		printf( "MegaIO id %d not detected\n", gHwAdd-0x31);
		exit(1);
	}
	if (argc == 3)
	{
		val = wiringPiI2CReadReg8(dev,GPIO_IT_FLAGS_MEM_ADD );
		printf("%d\n", val);
	}
	else
	{
  		printf( "Invalid command\n");
		exit(1);
	}
 } 
 
 /*
* doOcOutWrite:
*	Write coresponding Open collector output channel
**************************************************************************************
*/

static void doOcOutWrite(int argc, char *argv[])
{
	int pin, val, dev, valR, valRmask, retry;

	if ((argc != 5) && (argc != 4))
	{
		printf( "Usage: megaio <id> owrite <oc number> <on/off> \n") ;
		printf( "Usage: megaio <id> owrite <oc reg value> \n") ;
    
		exit (1) ;
	}
  
	dev = doBoardInit (gHwAdd);
	if(dev <= 0)
	{
		printf( "MegaIO id %d not detected\n", gHwAdd-0x31);
		exit(1);
	}
	if(argc == 5)
	{
		pin = atoi (argv [3]) ;
		if((pin < 1) || (pin > 4))
		{
			printf( "Open collector output number value out of range\n");
			exit(1);
		}
  
		if ((strcasecmp (argv [4], "up") == 0) || (strcasecmp (argv [4], "on") == 0))
			val = 1 ;
		else if ((strcasecmp (argv [4], "down") == 0) || (strcasecmp (argv [4], "off") == 0))
			val = 0 ;
		else
		{
			val = atoi (argv [4]) ;
		}
		if (val == 0)
		{
			wiringPiI2CWriteReg8 (dev,OC_OUT_CLR_MEM_ADD, pin);
		}
		else
		{
			wiringPiI2CWriteReg8 (dev,OC_OUT_SET_MEM_ADD, pin);
		}
		valR = wiringPiI2CReadReg8(dev, OC_OUT_VAL_MEM_ADD);
		valRmask = 0x01 & (valR >> (pin - 1));
		retry = RETRY_TIMES;
	
		while((retry > 0) && (valRmask != val))
		{
			if (val == 0)
			{
				wiringPiI2CWriteReg8 (dev,OC_OUT_CLR_MEM_ADD, pin);
			}
			else
			{
				wiringPiI2CWriteReg8 (dev,OC_OUT_SET_MEM_ADD, pin);
			}
			valR = wiringPiI2CReadReg8(dev, OC_OUT_VAL_MEM_ADD);
			valRmask = 0x01 & (valR >> (pin - 1));
			retry--;
		}
#ifdef DEBUG_I
		if(retry < RETRY_TIMES)
		{
			printf("retry %d times\n", 3-retry);
		}
#endif
		if(retry == 0)
		{
			printf( "Fail to write open collector output\n");
			exit(1);
		}
	}
	else
	{
		val = atoi (argv [3]) ;
		if(val < 0 || val > 0x0f)
		{
			printf( "Invalid open collector output value\n");
			exit(1);
		}
		retry = RETRY_TIMES;
		while((retry > 0) && (valR != val))
		{
			wiringPiI2CWriteReg8 (dev,OC_OUT_VAL_MEM_ADD, val);
		
			valR = wiringPiI2CReadReg8(dev, OC_OUT_VAL_MEM_ADD);
		}
		if(retry == 0)
		{
			printf( "Fail to write open collector output out\n");
			exit(1);
		}
	}
//wrLeds(gHwAdd-0x31, valR);
}


/*
* doOcOutRead:
*	Read Open collector output state
******************************************************************************************
*/
static void doOcOutRead(int argc, char *argv[])
{
  int pin, val, dev;

  dev = doBoardInit (gHwAdd);
  if(dev <= 0)
  {
    printf( "MegaIO id %d not detected\n", gHwAdd-0x31);
    exit(1);
  }
  
  if (argc == 4)
  {
    pin = atoi (argv [3]) ;
    if((pin < 1) || (pin > 4))
    {
      printf( "Open collector output number value out of range\n");
      exit(1);
    }

    val = wiringPiI2CReadReg8(dev, OC_OUT_VAL_MEM_ADD);
    val = val & (1 << (pin-1));
    if(val != 0)
    {
      printf("1\n");
    }
    else
    {
      printf("0\n");
    }
  }
  else if(argc == 3)
  {
    val = wiringPiI2CReadReg8(dev, OC_OUT_VAL_MEM_ADD);
    printf("%d\n", val);
  }
  else
  {
    printf( "Usage: %s read Open collector output value\n", argv [0]) ;
    exit (1) ;
  }
  
}

 
void doHelp(int argc, char *argv[])
{
	if(argc == 3)
	{
		/**/ if (strcasecmp (argv [2], "rwrite"    ) == 0)	
		{ 
			printf("\trwrite:      Set Relay On/Off\n");
			printf("\tUsage:       megaio <id> rwrite <channel> <on/off>\n");
			printf("\tExample:     megaio 0 rwrite 2 On; Set Relay #2 on Board #0 On\n"); 
		}
		else if (strcasecmp (argv [2], "board"     ) == 0)	
		{ 
			printf("\tboard:       Check MegaIO Board Hardware and Software Version\n");
			printf("\tUsage:       megaio <id> board\n");
			printf("\tExample:     megaio 0 board\n"); 
		}
		else if (strcasecmp (argv [2], "rread"     ) == 0)	
		{ 
			printf("\trread:       Read Relay Status\n");
			printf("\tUsage:       megaio <id> rread <channel>\n");
			printf("\tUsage:       megaio <id> rread\n");
			printf("\tExample:     megaio 0 rread 2; Read Status of Relay #2 on Board #0\n"); 
		}
		else if (strcasecmp (argv [2], "aread"     ) == 0)	
		{ 
			printf("\taread:       Read Analog Input\n");
			printf("\tUsage:       megaio <id> aread <channel>\n");
			printf("\tExample:     megaio 0 aread 2; Read Value of ADC Port #3\n"); 
		}
		else if (strcasecmp (argv [2], "awrite"    ) == 0)	
		{ 
			printf("\tawrite:      Write Analog Output\n");
			printf("\tUsage:       megaio <id> awrite <value>\n");
			printf("\tExample:     megaio 0 awrite 1024; Set DAC output to (1024/4095) of full scale\n"); 
		}
		else if (strcasecmp (argv [2], "optread"   ) == 0)	
		{ 
			printf("\toptread:     Read Optically Isolated Input bit or port\n");
			printf("\tUsage:       megaio <id> optread (<channel>)\n");
			printf("\tExample:     megaio 0 optread; Read all optically isolated inputs\n"); 
			printf("\tExample:     megaio 0 optread 3; Read optically isolated input #3\n"); 
		}
		else if (strcasecmp (argv [2], "optirqset" ) == 0)	
		{ 
			printf("\toptirqset:   Set Interrupt on Optically Isolated Input\n");
			printf("\tUsage:       megaio <id> optirqset <channel> <rising/falling/change/none>\n");
			printf("\tExample:     megaio 0 optirqset 5 rising; Interrupt on optoisolated channel 5 rising edge\n"); 
		}
		else if (strcasecmp (argv [2], "optitread" ) == 0)	
		{ 
			printf("\toptitread:   Read the pending interrupt(s) on Opto Isolated Input\n");
			printf("\tUsage:       megaio <id> optitread\n");
			printf("\tExample:     megaio 0 optitread\n"); 
		}
		else if (strcasecmp (argv [2], "iodread"   ) == 0) 
		{ 
			printf("\tiodread:     Read direction of IO Port(s)\n");
			printf("\tUsage:       megaio <id> iodread <channel>; Channel = 1..18\n");
			printf("\tExample:     megaio 0 iodread 7; Read direction of XPIO #7\n"); 
			printf("\tUsage:       megaio <id> iodread; Read direction of all XPIO ports\n");
			printf("\tExample:     megaio 0 iodread\n");
		}
		else if (strcasecmp (argv [2], "iodwrite"   ) == 0) 
		{ 
			printf("\tiodwrite:    Set direction of IO Port(s)\n");
			printf("\tUsage:       megaio <id> iodwrite <channel> <in/out>\n");
			printf("\tExample:     megaio 0 iodwrite 2 out; Set IO Port #2 Output\n"); 
		}
		else if (strcasecmp (argv [2], "iowrite"   ) == 0)	
		{ 
			printf("\tiowrite:     Set state of IO Port(s)\n");
			printf("\tUsage:       megaio <id> iowrite <channel> <0/1 or low/high>\n");
			printf("\tExample:     megaio 0 iowrite 3 1; Set XPIO 3 to high state\n"); 
		}
		else if (strcasecmp (argv [2], "ioread"    ) == 0)	
		{ 
			printf("\tioread:      Read state of IO Port(s)\n");
			printf("\tUsage:       megaio <id> ioread <channel>\n");
			printf("\tExample:     megaio 0 ioread 4 : Read the XPIO pin 4 State\n"); 
			printf("\tUsage:       megaio <id> ioread\n");
			printf("\tExample:     megaio 0 ioread; Read the entire XPIO port value\n"); 
		}
		else if (strcasecmp (argv [2], "ioirqset"    ) == 0)	
		{ 
			printf("\tioirqset:    Set Interrupt on XPIO Pin\n");
			printf("\tUsage:       megaio <id> ioirqset <channel> <rising/falling/change/none>\n");
			printf("\tExample:     megaio 0 ioirqset 5 change ; set on change interrupt on XPIO Pin 5\n"); 
		}
		else if (strcasecmp (argv [2], "ioitread"    ) == 0)	
		{ 
			printf("\tioitread:    Read the pending interrupt(s) on XPIO Pin(s)\n");
			printf("\tUsage:       megaio <id> ioitread\n");
			printf("\tExample:     megaio 0 ioitread\n"); 
		}
		else if (strcasecmp(argv[2], "-lt") == 0)
		{
			printf("\t-lt:         Test all the LED's \n");
			printf("\tUsage:       megaio -lt\n");
			printf("\tExample:     megaio -lt\n"); 
		}
		else if(strcasecmp(argv[2], "-lw") == 0)
		{
			printf("\t-lw:		Turn on/off LED's\n");
			printf("\tUsage:    megaio -lw <led_nr> <on/off; 1/0> : turn on/off the 'led_nr' LED\n");
			printf("\tExample:	megaio -lw 10 on\n");
			printf("\tUsage:    megaio -lw <val> : hex value of all 32 LED's\n");
			printf("\tExample:	megaio -lw 55555555 : turn on all odd number LED's\n");
		}
		else if(strcasecmp(argv[2], "atest") == 0)
		{
			printf("\tatest:  	Test a adc channel, compute mean, peak to peak and stdDev\n");
			printf("\tUsage:    megaio <id> atest <ch>\n");
			printf("\tExample:	megaio 0 atest 2 : test ADC channel 2\n");
		}
		else if(strcasecmp(argv[2], "test") == 0)
		{
			printf("\ttest:   \tTest the board, it pass only with test card connected\n");
			printf("\tWARNING:\tThis option should not be run with your board connected to external devices\n");
			printf("\tUsage:  \tmegaio <id> test\n");
			printf("\tExample:\tmegaio 0 test\n");
		}
		else if(strcasecmp(argv[2], "ocread") == 0)
		{
			printf("\tocread:   Read the open-collector output's\n");
			printf("\tUsage:    megaio <id> ocread\n");
			printf("\tExample:  megaio 0 ocread\n");
		}
		else if(strcasecmp(argv[2], "ocwrite") == 0)
		{
			printf("\tocwrite:  Write the open-collectot output's\n");
			printf("\tUsage:    megaio <id> ocwrite <val>\n");
			printf("\tExample:  megaio 0 ocwrite 5\n");
			printf("\tUsage:    megaio <id> ocwrite <ch> <val>\n");
			printf("\tExample:  megaio 0 ocwrite 2 on\n");
		}
	}
	else
	{
		printf ("%s: %s\n", argv [0], usage) ;
	}
}
		
void doBoard(int argc)
{
	int dev, hwMajor, hwMinor, minor , major;
	
	if(argc == 3)
	{
		dev = doBoardInit(gHwAdd);
		if(dev <= 0)
		{
			printf( "MegaIO id %d not detected\n", gHwAdd-0x31);
			exit(1);
		}
		hwMajor = wiringPiI2CReadReg8(dev, REVISION_HW_MAJOR_MEM_ADD);
		hwMinor = wiringPiI2CReadReg8(dev, REVISION_HW_MINOR_MEM_ADD);
		major = wiringPiI2CReadReg8(dev, REVISION_MAJOR_MEM_ADD);
		minor = wiringPiI2CReadReg8(dev, REVISION_MINOR_MEM_ADD);
		printf("MegaIO Hardware version %d.%d Firmware version %d.%d\n", hwMajor, hwMinor, major, minor);
	}
	else
	{
		printf( "Usage: megaio <id> board\n");
	}
}

static void doVersion(void)
{
	printf("MegaIO v%d.%d.%d Copyright (c) 2016 - 2017 Sequent Microsystems\n", VERSION_BASE, VERSION_MAJOR, VERSION_MINOR);
	printf("\nThis is free software with ABSOLUTELY NO WARRANTY.\n");
	printf("For details type: megaio -warranty\n");

}

static void  doAdcTest(int argc, char* argv[])
{
	int dev = -1;
	u16 adcVal[500] = {0};
	u16 minVal = 4096;
	u16 maxVal = 0;
	int chNr = 0;
	u16 addr;
	float mean = 0;
	float sigma = 0;
	int i;
	
	dev = doBoardInit (gHwAdd);
	if(dev <= 0)
	{
		printf( "MegaIO id %d not detected\n", gHwAdd-0x31);
		exit(1);
	}
	if(argc != 4)
	{
		printf("Invalid param numbers\n");
		exit(1);
	}
	chNr = atoi(argv[3]);
	if((chNr < 1) || (chNr > 8))
	{
		printf("Invalid channel number\n");
		exit(1);
	}
	addr = ADC_VAL_MEM_ADD + 2* (chNr-1);
	for(i = 0; i < 500; i++)
	{
		adcVal[i] = readReg16(dev, addr);
		if(adcVal[i] > maxVal)
			maxVal = adcVal[i];
		if(adcVal[i] < minVal)
			minVal = adcVal[i];
		mean += adcVal[i];
		
		delay(10);
		printf("\b\b\b\b\b");
		printf("%d", i);
		fflush(stdout);
	}
	printf("\n");
	mean = mean/500.0;
	for(i = 0; i< 500; i++)
	{
		sigma += (adcVal[i] - mean) * (adcVal[i] - mean);
	}
	sigma = sqrt(sigma / 500.0);
	printf("Statistics on ch %d : \n", chNr);
	printf("Standard deviation: %f\n", sigma);
	printf("Mean %d\n", (int)mean);
	printf("Peak to peak : %d\n", maxVal - minVal);	
}

/* 
* Self test for production
*/
static void doTest(int argc, char* argv[])
{
	int dev, i, retry;
	u8 relVal, valR, addr;
	int relayResult = 0;
	u16 adcVal;
	u16 dacVal = 0;
	u8 dacFault = 0;
	FILE* file = NULL;
	const u8 relayOrder[8] = {1, 2, 5, 6, 7, 8, 4, 3};
	 char* optTest[4] = {
		 "Open collector ch 1, opto ch 6 & 3 test",
		 "Open collector ch 2, opto ch 5 & 4 test",
		 "Open collector ch 3, opto ch 7 & 1 test",
		 "Open collector ch 4, opto ch 8 & 2 test"
	 };
	 u8 optoTab[4] = {0x24, 0x18, 0x41, 0x82};
	 u8 pass = 1;
	 u8 ocCh, opto;
	 int ioRead = 0;
	 
	dev = doBoardInit (gHwAdd);
	if(dev <= 0)
	{
		printf( "MegaIO id %d not detected\n", gHwAdd-0x31);
		exit(1);
	}
	if(argc == 4)
	{
		file = fopen( argv[3], "w");
		if(!file)
		{
			printf( "Fail to open result file\n");
			//return -1;
		}
		//fseek(file, SEEK_SET);
	}
	
//relay test****************************	
	if (strcasecmp( argv[2], "test") == 0)
	{
		relVal = 0;
		// Setup wiringPi
		printf("Is relay perform corectly? Press y for yes or any key for no....");
		wiringPiSetupSys ();
		 piThreadCreate (waitForKey);
		while(relayResult == 0)
		{
			
			for (i = 0; i < 8; i++)
			{
				piLock (COUNT_KEY) ;
				relayResult = globalResponse;
				piUnlock(COUNT_KEY);
				if(relayResult != 0)
				{
					break;
				}
				valR = 0;
				relVal = (u8)1<< (relayOrder[i] -1);
			
				retry = RETRY_TIMES;
				while((retry > 0) && ((valR & relVal) == 0))
				{
					wiringPiI2CWriteReg8 (dev,RELAY_ON_MEM_ADD, relayOrder[i]);	
					valR = wiringPiI2CReadReg8(dev, RELAY_MEM_ADD);
				}
				if(retry == 0)
				{
					printf( "Fail to write relay\n");
					if(file)
						fclose(file);
					exit(1);
				}		
				delay(150);
			}
			//delay(10);
		
			for (i = 0; i < 8; i++)
			{	
				piLock (COUNT_KEY) ;
				relayResult = globalResponse;
				piUnlock(COUNT_KEY);
				if(relayResult != 0)
				{
					break;
				}
				
				valR = 0xff;
				relVal = (u8)1<< (relayOrder[i] -1);
				retry = RETRY_TIMES;
				while((retry > 0) && ((valR & relVal) != 0))
				{
					wiringPiI2CWriteReg8 (dev,RELAY_OFF_MEM_ADD, relayOrder[i]);
					valR = wiringPiI2CReadReg8(dev, RELAY_MEM_ADD);
				}
				if(retry == 0)
				{
					printf( "Fail to write relay!\n");
					if(file)
						fclose(file);
					exit(1);
				}
				
				delay(150);
			}
		}
		
		wiringPiI2CWriteReg8 (dev,RELAY_MEM_ADD, 0x00);
		delay(150);
		if(relayResult == YES)
		{
			if(file)
			{
				fprintf(file, "Relay Test ............................ PASS\n");
			}
			else
			{
				printf("Relay Test ............................ PASS\n");
			}
		}
		else
		{
			if(file)
			{
				fprintf(file, "Relay Test ............................ FAIL!\n");
			}
			else
			{
				printf("Relay Test ............................ FAIL!\n");
			}
		}
		
// end relay test 
	}
	
// adc test ********************************************
	for(i = 1; i< 9;i++)
	{
		if(i == 7)
		{
			retry = 0;
			dacVal = 1;
			adcVal = 200;
			while (((dacVal != 0)|| (adcVal > 150)) && (retry < 100))
			{
				writeReg16(dev, DAC_VAL_H_MEM_ADD, 0x0000);
				delay(2); 
				dacVal = readReg16(dev,DAC_VAL_H_MEM_ADD);
				delay(2);
				retry ++;
				addr = ADC_VAL_MEM_ADD + 2* (i-1);
				adcVal = readReg16(dev, addr);
				delay(2);
			}
			addr = ADC_VAL_MEM_ADD + 2* (i-1);
			adcVal = readReg16(dev, addr);
			if(adcVal > 150)
			{
				if(file)
				{
					fprintf(file, "ADC ch 7 / DAC test ................... FAIL!: %d\n", adcVal);
				}
				else
				{
					printf("ADC ch 7 / DAC test ................... FAIL!: %d\n", adcVal);
					printf("DAC Retry %d\n", retry);
					pass = 0;
				}
				dacFault++;
				continue;
			}
			retry = 0;
			while (((dacVal != 3000)|| (adcVal < ADC_TEST_VAL_LOW)) && (retry < 100))
			{
				writeReg16(dev, DAC_VAL_H_MEM_ADD, 3000);
				delay(2);
				dacVal = readReg16(dev,DAC_VAL_H_MEM_ADD);
				delay(2);
				retry ++;
				addr = ADC_VAL_MEM_ADD + 2* (i-1);
				adcVal = readReg16(dev, addr);
			}
		}
		addr = ADC_VAL_MEM_ADD + 2* (i-1);
		delay(1);
		adcVal = readReg16(dev, addr);
		retry = 0;
		while(((ADC_TEST_VAL_LOW > adcVal) || (adcVal > ADC_TEST_VAL_HIGH)) && (retry < 100))
		{
			delay(2);
			adcVal = readReg16(dev, addr);
			retry ++;
		}
		
		if((ADC_TEST_VAL_LOW < adcVal) && (adcVal < ADC_TEST_VAL_HIGH))
		{
			if(file)
			{
				fprintf(file, "ADC ch %d test ......................... PASS\n", (int)i);
			}
			else
			{
				printf("ADC ch %d test ......................... PASS\n", (int)i);
			}
				
		}
		else
		{
			if(i == 7)
			{
				dacFault++;
				printf("DAC Retry %d\n", retry);
			}
			
			if(file)
			{
				fprintf(file, "ADC ch %d test ......................... FAIL!\n", (int)i);
			}
			else
			{
				printf("ADC ch %d test ......................... FAIL! val= %d\n", (int)i, (int)adcVal);
				pass = 0;
			}
		}		
	}
	if(dacFault == 0)
	{
		if(file)
		{
			fprintf(file, "DAC test .............................. PASS\n");
		}
		else
		{
			printf( "DAC test .............................. PASS\n");
		}
	}
	else
	{
		if(file)
		{
			fprintf(file, "DAC test .............................. FAIL!\n");
		}
		else
		{
			printf( "DAC test .............................. FAIL!\n");
			pass = 0;
		}
	}
	//*****************Open collector out/ Optocupled in test****************************/
	
	for(ocCh = 1; ocCh < 5; ocCh++)
	{
#ifdef TEST_RESET
	printf("hit a key\n");
	getchar();
#endif
		retry = 0;
		opto = optoTab[ocCh - 1] + 1;
		while((opto != optoTab[ocCh - 1]) && (retry < 10))
		{
			//delay(1);
			wiringPiI2CWriteReg8 (dev,OC_OUT_SET_MEM_ADD, ocCh);
			//delay(1);
			opto = wiringPiI2CReadReg8(dev, OPTO_IN_MEM_ADD);
			retry ++;
		}
		if(opto == optoTab[ocCh - 1])
		{
			//sprintf(outTest, "%s PASS\n", optTest[ocCh -1]);
			if(file)
			{
				fprintf(file, "%s PASS\n", optTest[ocCh -1]);
			}
			else
			{
				printf("%s PASS\n", optTest[ocCh -1]);
			}
		}
		else
		{
			//sprintf(outTest, "%s FAIL\n", optTest[ocCh -1]);
			if(file)
			{
				fprintf(file, "%s FAIL!\n", optTest[ocCh -1]);
			}
			else
			{
				printf("%s FAIL!\n", optTest[ocCh -1]);
				pass = 0;
			}
			//printf("ch %d Fail\n", (int)ocCh);
		}
		
		
		delay(1);
		wiringPiI2CWriteReg8 (dev,OC_OUT_CLR_MEM_ADD, ocCh);	
	}
	// ********************** I/O test*********************************************
	//setup io1/2/4 as output.
#ifdef TEST_RESET
	printf("hit a key\n");
	getchar();
#endif
	retry = 0;
	ioRead = 1;
	while((ioRead != 0) && (retry < 10))
	{
		wiringPiI2CWriteReg8 (dev,GPIO_DIR_MEM_ADD, 0x34);
		delay(1);
		wiringPiI2CWriteReg8 (dev,GPIO_VAL_MEM_ADD, 0);
		delay(1);
	
		ioRead = wiringPiI2CReadReg8(dev, GPIO_VAL_MEM_ADD);
		delay(1);
		retry ++;
	}
	dacFault = 0;
	
	if(ioRead != 0)
	{
		if(file)
		{
			fprintf(file, "IO test ............................... FAIL!: %d\n", ioRead);
		}
		else
		{
			printf("IO test ............................... FAIL!: %d\n", ioRead);
			pass = 0;
		}
	}
	else
	{
#ifdef TEST_RESET
	printf("hit a key\n");
	getchar();
#endif
		retry = 0;
		while((ioRead != 5) && (retry < 10))
		{
			delay(1);
			wiringPiI2CWriteReg8 (dev,GPIO_SET_MEM_ADD, 1);
			delay(1);
			ioRead =wiringPiI2CReadReg8(dev, GPIO_VAL_MEM_ADD);
			retry ++;
		}
		if(ioRead != 5)
		{
			if(file)
			{
				fprintf(file, "IO ch 1/3 test ........................ FAIL!: %d\n", ioRead);
			}
			else
			{
				printf("IO ch 1/3 test ........................ FAIL!: %d\n", ioRead);
				pass = 0;
			}
		}
		else
		{
			if(file)
			{
				fprintf(file, "IO ch 1/3 test ........................ PASS\n");
			}
			else
			{
				printf("IO ch 1/3 test ........................ PASS\n");
			}
		}
#ifdef TEST_RESET
	printf("hit a key\n");
	getchar();
#endif	
		retry = 0;
		while((ioRead != 18) && (retry < 10))
		{	
			delay(1);
			wiringPiI2CWriteReg8 (dev,GPIO_CLR_MEM_ADD, 1);
			delay(1);
			wiringPiI2CWriteReg8 (dev,GPIO_SET_MEM_ADD, 2);
			delay(1);
			ioRead = wiringPiI2CReadReg8(dev, GPIO_VAL_MEM_ADD);
			retry ++;
		}
		
		if(ioRead != 18)
		{
			if(file)
			{
				fprintf(file, "IO ch 2/5 test ........................ FAIL!: %d\n", ioRead);
			}
			else
			{
				printf("IO ch 2/5 test ........................ FAIL!: %d\n", ioRead);
				pass = 0;
			}
		}
		else
		{
			if(file)
			{
				fprintf(file, "IO ch 2/5 test ........................ PASS\n");
			}
			else
			{
				printf("IO ch 2/5 test ........................ PASS\n");
			}
		}
#ifdef TEST_RESET
	printf("hit a key\n");
	getchar();
#endif	
		retry = 0;
		while((ioRead != 40) && (retry < 10))
		{
			delay(1);
			wiringPiI2CWriteReg8 (dev,GPIO_CLR_MEM_ADD, 2);
			delay(1);
			wiringPiI2CWriteReg8 (dev,GPIO_SET_MEM_ADD, 4);
			delay(1);
			ioRead = wiringPiI2CReadReg8(dev, GPIO_VAL_MEM_ADD);
			delay(1);
			retry ++;
		}
		
		if(ioRead != 40)
		{
			if(file)
			{
				fprintf(file, "IO ch 4/6 test ........................ FAIL!: %d\n", ioRead);
			}
			else
			{
				printf("IO ch 4/6 test ........................ FAIL!: %d\n", ioRead);
				pass = 0;
			}
		}
		else
		{
			if(file)
			{
				fprintf(file, "IO ch 4/6 test ........................ PASS\n");
			}
			else
			{
				printf("IO ch 4/6 test ........................ PASS\n");
			}
		}
#ifdef TEST_RESET
	printf("hit a key\n");
	getchar();
#endif
		wiringPiI2CWriteReg8 (dev,GPIO_CLR_MEM_ADD, 4);
	}
	delay(1);
	wiringPiI2CWriteReg8 (dev,GPIO_DIR_MEM_ADD, 0x3f);	
	if(file)
	{
		fclose(file);
	}
	/*
	if(pass == 1)
	{
		printf("╔═══╦═══╦═══╦═══╗\n");
		printf("║╔═╗║╔═╗║╔═╗║╔═╗║\n");
		printf("║╚═╝║║─║║╚══╣╚══╗\n");
		printf("║╔══╣╚═╝╠══╗╠══╗║\n");
		printf("║║──║╔═╗║╚═╝║╚═╝║\n");
		printf("╚╝──╚╝─╚╩═══╩═══╝\n");
	}
	else
	{
		printf("╔═══╗╔═══╗╔══╗╔╗───╔╗\n");
		printf("║╔══╝║╔═╗║╚╣╠╝║║───║║\n");
		printf("║╚══╗║║─║║─║║─║║───║║\n");
		printf("║╔══╝║╚═╝║─║║─║║─╔╗╚╝\n");
		printf("║║───║╔═╗║╔╣╠╗║╚═╝║╔╗\n");
		printf("╚╝───╚╝─╚╝╚══╝╚═══╝╚╝\n");
	}
	*/
}

/*
* Main loop
************************************************************************************************
*/
int main(int argc, char *argv [])
{
  int id;
  
  if (argc == 1)
  {
    printf( "%s\n", usage) ;
    return 1 ;
  }

  
// Help

  if (strcasecmp (argv [1], "-h") == 0)
  {
    doHelp(argc, argv);
    return 0;
  }
  
  // Warranty

  if (strcasecmp (argv [1], "-warranty") == 0)
  {
    printf("%s\n", warranty);
    return 0;
  }
// Version

  if (strcasecmp (argv [1], "-v") == 0)
  {
    doVersion();
    return 0;
  }


  if (strcasecmp (argv [1], "-lt") == 0)
  {
    doLedTest();
    return 0;
  }
  
  if (strcasecmp (argv [1], "-lw") == 0)
  {
    doLed(argc, argv);
    return 0;
  }
  
  if(argc < 3)
  {
	printf( "%s\n", usage) ;
    return 1 ;
  }
  id = atoi(argv[1]);
  if((id < 0) || (id > 3))
  {
    printf( "invalid boartd id\n");
	return 1;
  }
  gHwAdd = 0x31 + id;
  
  
  /**/ if (strcasecmp (argv [2], "rwrite"    ) == 0)	{ doRelayWrite     (argc, argv) ;	return 0 ; }
  else if (strcasecmp (argv [2], "rread"     ) == 0)	{ doRelayRead      (argc, argv) ;	return 0 ; }
  else if (strcasecmp (argv [2], "aread"     ) == 0)	{ doAnalogRead     (argc, argv) ;	return 0 ; }
  else if (strcasecmp (argv [2], "awrite"    ) == 0)	{ doAnalogWrite    (argc, argv) ;	return 0 ; }
  else if (strcasecmp (argv [2], "optread"   ) == 0)	{ doOptoInRead     (argc, argv) ;	return 0 ; }
  else if (strcasecmp (argv [2], "optirqset" ) == 0)	{ doOptoInIrq      (argc, argv) ;	return 0 ; }
  else if (strcasecmp (argv [2], "optitread" ) == 0)	{ doOptoInIt       (argc) 		;	return 0 ; }
  else if (strcasecmp (argv [2], "iodwrite"  ) == 0)	{ doIoModeSet      (argc, argv) ;	return 0 ; }
  else if (strcasecmp (argv [2], "iodread"   ) == 0)	{ doIoModeGet      (argc, argv) ;	return 0 ; }
  else if (strcasecmp (argv [2], "iowrite"   ) == 0)	{ doIoSet          (argc, argv) ;	return 0 ; }
  else if (strcasecmp (argv [2], "ioread"    ) == 0)	{ doIoGet          (argc, argv) ;	return 0 ; }
  else if (strcasecmp (argv [2], "ioirqset"  ) == 0)	{ doIoIrqSet       (argc, argv) ;	return 0 ; }
  else if (strcasecmp (argv [2], "ioitread"  ) == 0)	{ doIoIt       	   (argc) 		;	return 0 ; }
  else if (strcasecmp (argv [2], "board"     ) == 0)	{ doBoard          (argc) 		;	return 0 ; } 
  else if (strcasecmp (argv [2], "ocwrite"   ) == 0)	{ doOcOutWrite     (argc, argv) ;	return 0 ; }
  else if (strcasecmp (argv [2], "ocread"    ) == 0)	{ doOcOutRead      (argc, argv) ;	return 0 ; }
  else if (strcasecmp (argv [2], "test"      ) == 0)	{ doTest           (argc, argv) ;	return 0 ; }
  else if (strcasecmp (argv [2], "testc"     ) == 0)	{ doTest           (argc, argv) ;	return 0 ; }
  else if (strcasecmp (argv [2], "atest"     ) == 0)	{ doAdcTest        (argc, argv) ;	return 0 ; }
  else { printf( "%s\n", usage) ; return 1;}
  return 0;
}
