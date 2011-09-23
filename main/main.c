#include <util.h>
#include <ts7200.h>
#include <rawio.h>
#include <hardware.h>
#include <task.h>

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
	int i= 0;
	bwprintf(COM2, "%x\n", &i);
//	install_interrupt_handlers();
//
//	ASM(
//			"swi 0" "\n\t"
//	);

	return 0;
}
