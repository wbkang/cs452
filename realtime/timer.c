
#include <timer.h>
#include <ts7200.h>
#include <util.h>


void timer3_init()
{
	// 2khz timer
	MEM(TIMER3_BASE + CRTL_OFFSET) &= ~CLKSEL_MASK;
	MEM(TIMER3_BASE + CRTL_OFFSET) |= ENABLE_MASK;
}

void timer_settimeout(void (*fn)(void*), void* arg, int timeoutms)
{

}

unsigned int timer3_getvalue()
{
	return 0xffffffff - MEM(TIMER3_BASE + VAL_OFFSET);
}

// busy sleep
void sleep(int ms)
{
	unsigned int cur_time = timer3_getvalue();
	// 2khz timer	
	unsigned int actual_tick = ms * 2;

	while (timer3_getvalue() < cur_time + actual_tick);

	return;
}
