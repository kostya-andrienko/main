/******************************************************************************
*
* helloworld.c: simple test application
*
* MODIFICATION HISTORY:
*
* Ver   Who      Date     Changes
* ----- -------- -------- -----------------------------------------------
* 0.1 	Kostya 	 29/12/21 First release
*
*
******************************************************************************/

#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "sleep.h"

int main()
{
	unsigned int n,i;

	#define NMAX 10

	init_platform();

	xil_printf("\r######################################## \n\r");

	xil_printf("CPU: ");

	#if defined (ARMR5)
		xil_printf("RPU R5");
	#elif defined (__aarch64__) || defined (ARMA53_32)
		xil_printf("APU A53");
	#elif defined (__MICROBLAZE__)
		xil_printf("MICROBLAZE");
	#else
		xil_printf("Unknown");
	#endif
		xil_printf(" \n\r");

	xil_printf("Start Hello World application \n\r");
	for(n=0; n<NMAX; n++)
	{
	  	xil_printf("Hello World #%02d\t[of %d] ", n+1, NMAX);
	   	for(i=0; i<10; i++)
	   	{
	   		usleep(100000);
	   		xil_printf(".");
	   	}
	   	xil_printf("\n\r");
	}
	xil_printf("Successfully ran Hello World application \n\n\r");
	cleanup_platform();
	return 0;
}
