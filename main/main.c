 /*
 * iotest.c
 */

#include <bwio.h>
#include <console.h>
#include <ts7200.h>
#include <util.h>

#define RTC_ADDR 	0x80920000
#define RTC_SWCOMP 	0x80920108


int main(int argc, char* argv[]) {
	bwsetspeed(COM1, 2400);
	bwsetfifo( COM1, OFF );
	bwsetfifo( COM2, OFF );

	int* uart1high = (int*) UART1_BASE + UART_LCRH_OFFSET;
	*uart1high |= STP2_MASK;

	argc = 10000;
	while(argc--);
	//MEM(TIMER3_BASE + CRTL_OFFSET) &= ~CLKSEL_MASK;
	//MEM(TIMER3_BASE + CRTL_OFFSET) |= ENABLE_MASK;

	//unsigned int time = MEM(TIMER3_BASE + VAL_OFFSET); 


	return 0;
}

