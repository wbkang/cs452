#include <util.h>
#include <ts7200.h>
#include <rawio.h>
#include <hardware.h>
#include <task.h>
#include <memory.h>

// asm
void asm_handle_swi();

void handle_swi(register_set reg, int service_no) {
	bwprintf(COM2, "Dumping registers...\n");
	for (int i = 0; i < 15; i++)
	{
		bwprintf(COM2, "r%d: %x\n", i, reg->registers[i]);
	}
}

static void install_interrupt_handlers() {
	INSTALL_INTERRUPT_HANDLER(0x8, asm_handle_swi);
}

int main(int argc, char* argv[]) {
	uart_fifo(COM2, OFF);
	
	install_interrupt_handlers();

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


	ASM(
			"swi 0" "\n\t"
	);

	return 0;
}
