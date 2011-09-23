#include <util.h>
#include <ts7200.h>
#include <rawio.h>
#include <hardware.h>
#include <memory.h>

// asm
void asm_handle_swi();

void handle_swi(int i) {
	while (!raw_istxready(COM2))
		;
	raw_putc(COM2, '0' + i);
}

static void install_interrupt_handlers() {
	INSTALL_INTERRUPT_HANDLER(0x8, asm_handle_swi);
}

int main(int argc, char* argv[]) {
	uart_fifo(COM2, OFF);

	pages_test();

	return 0;

	install_interrupt_handlers();

	ASM( "swi 0" "\n\t"
	"swi 1" "\n\t"
	"swi 2" "\n\t"
	"swi 3" "\n\t"
	"swi 4" "\n\t"
	"swi 5" "\n\t"
	"swi 6" "\n\t"
	"swi 7" "\n\t"
	"swi 8" "\n\t"
	"swi 9" "\n\t");

	return 0;
}
