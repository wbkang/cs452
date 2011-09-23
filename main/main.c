 /*
 * iotest.c
 */


#include <util.h>
#include <ts7200.h>
#include <rawio.h>

// asm
void asm_handle_swi();

void handle_swi(int i)
{
	while(!raw_istxready(COM2));
	raw_putc(COM2, '0' + i);
}

static void install_interrupt_handlers()
{
	//unsigned int addr = (unsigned int) asm_handle_swi;

	INSTALL_INTERRUPT_HANDLER(0x8, asm_handle_swi);
	//INSTALL_INTERRUPT_HANDLER(0x8, 0x21805c);

	//MEM(0x8) = 0xea ;
}

int main(int argc, char* argv[]) {
	MEM(UART2_BASE + UART_LCRH_OFFSET) &= ~FEN_MASK;
	install_interrupt_handlers();	
	
	__asm(
#if 1
		"swi 0" "\n\t"
#endif
		"swi 1" "\n\t"
#if 1
		"swi 2" "\n\t"
		"swi 3" "\n\t"
		"swi 4" "\n\t"
		"swi 5" "\n\t"
		"swi 6" "\n\t"
		"swi 7" "\n\t"
		"swi 8" "\n\t"
		"swi 9" "\n\t"
		"swi 10" "\n\t"
#endif
	);
	
	return 0;
}
