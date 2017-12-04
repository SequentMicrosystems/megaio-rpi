/*
 * wfi.c:
 *	Wait for Interrupt test program
 *
 *	This program demonstrates the use of the megaio interrupt usage
 *	
 *	BCM_GPIO pin 4 used for interrupt the raspberry
 *
 *	The biggest issue with this method is that it really only works
 *	well in Sys mode.
 *
 ***********************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wiringPi.h>

// A 'key' which we can lock and unlock - values are 0 through 3
//	This is interpreted internally as a pthread_mutex by wiringPi
//	which is hiding some of that to make life simple.

#define	COUNT_KEY	0

// What BCM_GPIO input are we using?

#define	BUTTON_PIN	4

// Debounce time in mS

#define	DEBOUNCE_TIME	100


// globalCounter:
//	Global variable to count interrupts
//	Should be declared volatile to make sure the compiler doesn't cache it.

static volatile int globalCounter = 0 ;


/*
 * waitForIt:
 *	This is a thread created using the wiringPi simplified threading
 *	mechanism. It will wait on an interrupt on the button and increment
 *	a counter.
 *********************************************************************************
 */

PI_THREAD (waitForIt)
{
 

  (void)piHiPri (10) ;	// Set this thread to be high priority

  for (;;)
  {
    if (waitForInterrupt (BUTTON_PIN, -1) > 0)	// Got it
    {
// state ^= 1 ;
	digitalRead (BUTTON_PIN);
      piLock (COUNT_KEY) ;
	++globalCounter ;
      piUnlock (COUNT_KEY) ;

    }
  }
}


/*
 * setup:
 *	Demo a crude but effective way to initialise the hardware
 *********************************************************************************
 */

void setup (void)
{
int i;
char str[250];

// Use the gpio program to initialise the hardware
//	(This is the crude, but effective)

  system ("gpio edge 4 both") ;
  /*for (i = 1; i< 19; i++)
  {
	  sprintf(str,"megaio 0 ioirqset %d none", i); 
	  printf("%s\n", str);
	system (str);
	
	delay(300);
	
	}*/
	
	for (i = 1; i< 9; i++)
	{
		sprintf(str,"megaio 0 optirqset %d change", i); 
		 printf("%s\n", str);
		system (str);
		delay(40);
	}
	
//*/
// Setup wiringPi

  wiringPiSetupSys () ;

// Fire off our interrupt handler

  piThreadCreate (waitForIt) ;

}



/*
 * main
 *********************************************************************************
 */

int main (void)
{
  int lastCounter = 0 ;
  int myCounter   = 0 ;

  setup () ;

  for (;;)
  {
    printf ("Waiting ... ") ; fflush (stdout) ;

    while (myCounter == lastCounter)
    {
      piLock (COUNT_KEY) ;
	  myCounter = globalCounter ;
      piUnlock (COUNT_KEY) ;
      delay (10) ;
    }

    //printf (" Done. myCounter: %5d\n", myCounter) ;
   
/*	printf ("IO interrupts flags ");
	fflush (stdout) ;
	system("megaio 0 ioitread");*/
	
	//delay(100);
	printf ("OPTO IN interrupts flags ");
	fflush (stdout) ;
	system("megaio 0 optitread");
	
    lastCounter = myCounter ;
  }

  return 0 ;
}
