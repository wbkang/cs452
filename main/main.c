 /*
 * iotest.c
 */

#include <bwio.h>
#include <console.h>
#include <util.h>
#include <train.h>
#include <timer.h>
#include <a0ui.h>


static int runtimer();
static int quit_signalled;

static void fptr_test()
{
	BOOTLOG("fptr_test: making sure the linker script is correct.");
}

static void flush_com1()
{
	unsigned int curtime = timer3_getvalue();
	BOOTLOG("flushing com1 input for a second");
	while(timer3_getvalue() < curtime + 1000) {
		if (bwtrygetc(COM1) >= 0)
		{
			BOOTLOG("*");	
		}
	}
	BOOTLOG("flushing com1 done");
}


int main(int argc, char* argv[]) {
	quit_signalled = 0;
	console_init();

	void (*x)() = fptr_test; x();

	BOOTLOG("TIMER INIT...");
	timer3_init();
	BOOTLOG("BWIO INIT...");
	bwinit();
	BOOTLOG("TRAIN INIT...");
	train_init();

	BOOTLOG("WAITING UNTIL ALL THE PACKETS ARE SENT...");
	while(bwtryputc(COM1) || bwtryputc(COM2) || runtimer());
	flush_com1();
	
	BOOTLOG("BOOTING DONE");
	

	a0ui_start();
	
	unsigned int lastkeystroke_time = 0;
	int sensorbuf = -1; // a 2 byte buffer for sensor information.

	while (!quit_signalled)
	{
		unsigned int curtime = timer3_getvalue();

		bwtryputc(COM2);
		bwtryputc(COM1);
		bwtryputc(COM2);
		bwtryputc(COM2);

		int key = bwtrygetc(COM2);

		if (key >= 0 /*&& curtime - lastkeystroke_time >= 10*/) {
			lastkeystroke_time = curtime;
			a0ui_handleKeyInput(key);
		}

		
		int sensor = bwtrygetc(COM1);

		if (sensor >= 0)
		{
			if(sensorbuf == -1)
			{
				sensorbuf = sensor << 8;
			}
			else 
			{
				sensorbuf |= sensor;
				a0ui_handleSensorInput(sensorbuf);
				sensorbuf = -1;
			}
		}

		runtimer();
	}
	
	
	while(bwtryputc(COM1) || bwtryputc(COM2));
	BOOTLOG("\033[0m\033c\033[2J" CRLF "GOODBYE!" CRLF);
	console_close();

	return 0;
}

static int runtimer()
{
	struct timeout_info timeout = timer_poptimeout();

	if (timeout.callback)
	{
		//ASSERT(0 < (int)timeout.callback, "WRONG JUMP ADDR");

		ASSERT(!(((unsigned int)timeout.callback) & 0xff000000), "WRONG JUMP ADDR1");
		((timeout_callback)(FPTR_OFFSET+(unsigned int)timeout.callback))(timeout.arg);
	}
	
	return timer_timeoutcount();
}

void signal_quit()
{
	quit_signalled = 1;
}
