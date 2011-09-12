 /*
 * iotest.c
 */

#include <bwio.h>
#include <ts7200.h>

#define RTC_ADDR 	0x80920000
#define RTCSWCOMP 	0x80920108
int main( int argc, char* argv[] ) {
	char str[] = "Hello\n\r";
	unsigned int prevValue = *(unsigned int*)RTC_ADDR;
	bwsetfifo( COM2, OFF );
//	bwputstr( COM2, str );
//	bwputw( COM2, 10, '*', str );
//	bwprintf( COM2, "Hello world.\n\r" );
//	bwprintf( COM2, "%s world%u.\n\r", "Well, hello", 23 );
//	bwprintf( COM2, "%d worlds for %u person.\n\r", -23, 1 );
//	bwprintf( COM2, "%x worlds for %d people.\n\r", -23, 723 );
//	str[0] = bwgetc( COM2 );
//	bwprintf( COM2, "%s", str );

	unsigned int swcomp = *(RTCSWCOMP);
	*swcomp = (swcomp&0xffff0000) | 0x3fff;

	while(1) {
		unsigned int curValue = *(unsigned int*)RTC_ADDR;

		if (curValue != prevValue)
		{
			bwprintf(COM2, "RTCDATA: %d\n\r", curValue);
			prevValue = curValue;
		}
	}

	return 0;
}

