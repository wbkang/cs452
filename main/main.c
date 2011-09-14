 /*
 * iotest.c
 */

#include <bwio.h>
#include <ts7200.h>

#define RTC_ADDR 	0x80920000
#define RTC_SWCOMP 	0x80920108

#define MEM(x) (*(unsigned int*)(x))
#define CONSOLE_OUT(x) bwputstr(COM2, x)
#define CONSOLE_CLEAR() CONSOLE_OUT("\033[2J")
#define CONSOLE_MOVE(line,column) bwprintf(COM2, "\033[%d;%dH", line, column)

int main(int argc, char* argv[]) {
	//char str[] = "Hello\n\r";
	bwsetspeed(COM1, 2400);
	//bwsetspeed(COM2, 2400);
	bwsetfifo( COM1, OFF );
	bwsetfifo( COM2, OFF );


	int* uart1high = (int*) UART1_BASE + UART_LCRH_OFFSET;
	*uart1high |= STP2_MASK;
	//*uart1high &= ~PEN_MASK;
	//*uart1high |= 0x60;

	argc = 10000;
	while(argc--);
//	bwputstr( COM2, str );
//	bwputw( COM2, 10, '*', str );
//	bwprintf( COM2, "Hello world.\n\r" );
//	bwprintf( COM2, "%s world%u.\n\r", "Well, hello", 23 );
//	bwprintf( COM2, "%d worlds for %u person.\n\r", -23, 1 );
//	bwprintf( COM2, "%x worlds for %d people.\n\r", -23, 723 );
//	str[0] = bwgetc( COM2 );
//	bwprintf( COM2, "%s", str );
	//MEM(TIMER3_BASE + CRTL_OFFSET) &= ~CLKSEL_MASK;
	//MEM(TIMER3_BASE + CRTL_OFFSET) |= ENABLE_MASK;

	while(0){
		bwputc(COM1,'1');
		bwputc(COM1, 10);
		bwputc(COM2,'2');
		unsigned int time = MEM(TIMER3_BASE + VAL_OFFSET); 
		while (time == MEM(TIMER3_BASE + VAL_OFFSET)); 
	}
	//while(1) {
		//unsigned int curValue = MEM(TIMER3_BASE + VAL_OFFSET); 

		while(1) {
			int keybuf = 0;

			bwputstr(COM2, "SENDING THE COMMAND: ");

			while(1){
				keybuf = bwgetc(COM2);
				bwputc(COM1, keybuf);
				bwprintf(COM2, "%x", keybuf);
				bwputstr(COM2, "\r\n");
			}

			//bwputstr(COM2, "\n\r");
			//bwputstr(COM1, "\n\r");
		}

		
		//CONSOLE_MOVE(11, 0);
	//}

	return 0;
}

