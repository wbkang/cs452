#include <kernel.h>
#include <assembly.h>
#include <hardware.h>
#include <util.h>
#include <rawio.h>
#include <task.h>

static void install_interrupt_handlers() {
	INSTALL_INTERRUPT_HANDLER(SWI_VECTOR, asm_handle_swi);
}

void handle_swi(register_set *reg, int service_no) {
	bwprintf(COM2, "Handling swi #%d", service_no);
	bwprintf(COM2, "Dumping registers...\n");
	for (int i = 0; i < 15; i++) {
		bwprintf(COM2, "r%d: %x\n", i, reg->registers[i]);
	}
}

void kernel_init() {
	install_interrupt_handlers();
}

int kernel_create(int priority, void(*code)()) {
	// test weather *code is valid
	// generate task id
	// initialize user stack
	// append task to scheduler ready queue
	// return my task id
	return 0;
}

int kernel_mytid() {
	// return my task id
	return 0;
}

int kernel_myparenttid() {
	// return my parents task id
	return 0;
}

void kernel_pass() {
	// put me on the ready queue
}

void kernel_exit() {
	// remove me from all queues and die
}
