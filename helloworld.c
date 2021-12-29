/******************************************************************************
*

*
******************************************************************************/

#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "sleep.h"

#define NMAX 60


int main()
{
	unsigned int n,i;

	init_platform();
	xil_printf("\r######################################## \n\r");
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
