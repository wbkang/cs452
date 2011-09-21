#include <util.h>
#include <ts7200.h>

int raw_isrxempty(int channel) {
	CHECK_COM(channel);
	return MEM(UART_BASE(channel) + UART_FLAG_OFFSET) & RXFE_MASK;
}

int raw_getc( int channel ) {
	CHECK_COM(channel);
	return MEM(UART_BASE(channel) + UART_DATA_OFFSET);
}

int raw_istxready(int channel) {
	CHECK_COM(channel);
	int flag = MEM(UART_BASE(channel) + UART_FLAG_OFFSET);
	return !(flag & TXFF_MASK) && (channel == COM2 ||  (channel == COM1 && (flag & CTS_MASK)));
}

void raw_putc(int channel, char c) {
	CHECK_COM(channel);
	MEM(UART_BASE(channel) + UART_DATA_OFFSET) = c;
}


int raw_putcc( int channel, char c ) {
	CHECK_COM(channel);
	int *flags, *data;
	switch( channel ) {
		case COM1:
			flags = (int *)( UART1_BASE + UART_FLAG_OFFSET );
			data = (int *)( UART1_BASE + UART_DATA_OFFSET );
			break;
		case COM2:
			flags = (int *)( UART2_BASE + UART_FLAG_OFFSET );
			data = (int *)( UART2_BASE + UART_DATA_OFFSET );
			break;
		default:
			return -1;
	}
	while( ( *flags & TXFF_MASK ) ) ;
	*data = c;
	return 0;
}

void raw_logemergency(int channel, char* c)
{
	CHECK_COM(channel);
	while (*c) {
		raw_putcc(channel, *c);
		c++;
	}
}
