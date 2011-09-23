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

	struct _tag_pages _p;
	pages p = &_p;
	memptr node_table[8];
	pages_init(p, 8, (uint) node_table, 0x300000);
	memptr test[8];
	int i;
	for (i = 7; i >= 0; --i) {
		test[i] = pages_get(p);
		bwprintf(COM2, "page %d: %x\n", i + 1, (uint) test[i]);
	}
	bwprintf(COM2, "head after remove 5: %x\n", (uint) p->head);
	for (i = 7; i >= 0; --i) {
		pages_put(p, test[7 - i]);
		bwprintf(COM2, "head %d: %x\n", i + 1, (uint) p->head);
	}

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
