#include <util.h>
#include <hardware.h>
#include <rawio.h>
#include <task.h>

void die() {
	for (;;) {
	}
}

void errormsg(char* msg) {
	bwprintf(COM2, msg);
}

void dump_registers(int r0, int r1, int r2, int r3, int r4) {
	TRACE("dump_registers:\n\tr0: %x\n\tr1: %x\n\tr2: %x\n\tr3: %x\n\tr4: %x\n", r0, r1, r2, r3, r4);
}
