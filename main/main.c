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


	logmsg("before initial timer run!");
	while(bwtryputc(COM1) || bwtryputc(COM2) || runtimer());
	console_printf("after intial timer run!" CRLF);

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
