 /*
 * iotest.c
 */

#include <bwio.h>
#include <console.h>
#include <ts7200.h>
#include <util.h>
#include <train.h>


#define RTC_ADDR 	0x80920000
#define RTC_SWCOMP 	0x80920108

#define pause() { int i = 1000000; while(i-->0); }


int main(int argc, char* argv[]) {
	console_init();
	train_init();

	argc = 10000;
	while(argc--);
	MEM(TIMER3_BASE + CRTL_OFFSET) &= ~CLKSEL_MASK;
	MEM(TIMER3_BASE + CRTL_OFFSET) |= ENABLE_MASK;

	//unsigned int time = MEM(TIMER3_BASE + VAL_OFFSET); 

	train_init();
	pause();
	train_go();
	pause();
	train_setspeed(21,14);
	pause();
	train_reverse(21);




	return 0;
}

