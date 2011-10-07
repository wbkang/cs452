#include <util.h>
#include <hardware.h>
#include <rawio.h>
#include <task.h>

void die() {
	for (;;);
}

void errormsg(char* msg) {
	bwprintf(COM2, msg);
}

void dump_registers(int r0, int r1, int r2, int r3) {
	TRACE("dump_registers:\n\tr0: %x\n\tr1: %x\n\tr2: %x\n\tr3: %x", r0, r1, r2, r3);
}

static uint random_seed;

uint random() {
	random_seed = random_seed * 1103515245 + 12345;
	return (uint) (random_seed / 65536) % 32768;
}
