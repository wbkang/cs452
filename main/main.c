 /*
 * iotest.c
 */

#include <bwio.h>
#include <console.h>
#include <ts7200.h>
#include <util.h>
#include <train.h>
#include <timer.h>
#include <rawio.h>



int main(int argc, char* argv[]) {
	console_init();
	train_init();
	timer3_init();

	bwinit();

	console_clear();
	console_move(0,0);
	
	//console_printf("TIMER TEST. Waiting %d seconds" CRLF, 3);

	//sleep(3 * 1000);
	console_printf("TESTABC");
	
	
	while (1)
	{
		if (raw_istxready(COM2))
		{
			int v = bwpopoutbuf(COM2);
			if (v != -1) {
				raw_putc(COM2, (char)v);
			}
		}

	//	sleep(1000);
	}
	

	return 0;
}

