 /*
 * iotest.c
 */

#include <bwio.h>
#include <ts7200.h>

#define RTC_ADDR 0x11700000

int main( int argc, char* argv[] ) {
	char str[] = "Hello\n\r";
	int prevValue = *(int*)RTC_ADDR;
	bwsetfifo( COM2, OFF );
//	bwputstr( COM2, str );
//	bwputw( COM2, 10, '*', str );
//	bwprintf( COM2, "Hello world.\n\r" );
//	bwprintf( COM2, "%s world%u.\n\r", "Well, hello", 23 );
//	bwprintf( COM2, "%d worlds for %u person.\n\r", -23, 1 );
//	bwprintf( COM2, "%x worlds for %d people.\n\r", -23, 723 );
//	str[0] = bwgetc( COM2 );
//	bwprintf( COM2, "%s", str );

	while(1) {
		int curValue = *(int*)RTC_ADDR;

		if (curValue != prevValue)
		{
			bwprintf(COM2, "RTCDATA: %d, %x\n\r", curValue, curValue);
		}
	}

	return 0;
}

