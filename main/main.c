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


int main(int argc, char* argv[]) {
	quit_signalled = 0;
	console_init();
	BOOTLOG("KERNEL INIT...");
	timer3_init();
	bwinit();
	train_go();
	train_init();

	BOOTLOG("INIT FINISHED!");
	BOOTLOG("INITIALIZING THE TRACK...");
	while(bwtryputc(COM1) || bwtryputc(COM2) || runtimer());
	BOOTLOG("DONE");


	a0ui_start();
	
	unsigned int lastkeystroke_time = 0;
	//int lastkeystroke = -1;
	unsigned int sensorbuf = -1;

	while (!quit_signalled)
	{
		unsigned int curtime = timer3_getvalue();

		bwtryputc(COM2);
		bwtryputc(COM1);
		bwtryputc(COM2);
		bwtryputc(COM2);

		int key = bwtrygetc(COM2);

		if (key > 0 && curtime - lastkeystroke_time >= 10) {
			lastkeystroke_time = curtime;

			a0ui_handleKeyInput(key);
		}

	//	lastkeystroke = key;
		
		int sensor = bwtrygetc(COM1);

		if (sensor >= 0)
		{
			if(sensorbuf == -1)
			{
				sensorbuf = sensor;
			}
			else 
			{
				sensorbuf |= sensor << 8;
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
		((timeout_callback)(FPTR_OFFSET+(unsigned int)timeout.callback))(timeout.arg);
	}
	
	return timer_timeoutcount();
}

void signal_quit()
{
	quit_signalled = 1;
}
