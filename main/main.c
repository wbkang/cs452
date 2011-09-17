 /*
 * iotest.c
 */

#include <bwio.h>
#include <console.h>
#include <util.h>
#include <train.h>
#include <timer.h>


static void timeout_test(void*);
static int runtimer();

int main(int argc, char* argv[]) {
	console_init();
	logmsg("KERNEL INIT...");
	timer3_init();
	bwinit();
	train_init();

	logmsg("KERNEL INIT FINISHED!");

	console_clear();
	console_move(0,0);
	console_printf("BOOTING....");
	logmsg("booting");
	while(bwtryputc(COM1) || bwtryputc(COM2) || runtimer());
	logmsg("finished booting");
	console_printf("DONE!" CRLF);

	console_effect(EFFECT_FG_RED);
	console_fillrect('$', 60, 10, 10, 10);

	console_effect(EFFECT_RESET);
	console_effect(EFFECT_BG_BLUE);
	console_printf("TEST" CRLF);

	console_effect(EFFECT_RESET);
	console_fillrect('$', 10, 10, 10, 10);

	console_move(20, 10);
	console_printf("HELLO WORLD 2010" CRLF);

	train_go();
	train_setspeed(21, 14);
	train_setspeed(24, 14);

	for (int i = 1; i < 10; i++)
	{
		timer_settimeout(timeout_test, 0, 10000 * i);
	}

	
	while (1)
	{
		bwtryputc(COM1);
		bwtryputc(COM2);
		runtimer();
	}
	

	return 0;
}

static int runtimer()
{
	struct timeout_info timeout = timer_poptimeout();

	if (timeout.callback)
	{
		((timeout_callback)(0x218000+(unsigned int)timeout.callback))(timeout.arg);
	}
	
	return timer_timeoutcount();
}


void timeout_test(void * a)
{
	unsigned int number = (unsigned int)a;

	console_printf("timeout_test: %u" CRLF, number);

	train_reverse(21);
	train_reverse(24);
}
