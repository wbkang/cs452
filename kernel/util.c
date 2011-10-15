#include <util.h>
#include <hardware.h>
#include <rawio.h>
#include <task.h>

void die() {
	for (;;);
}

void errormsg(char *msg) {
	bwprintf(COM2, msg);
}

void dump_registers(int r0, int r1, int r2, int r3) {
	TRACE("dump_registers:\n\tr0: %x\n\tr1: %x\n\tr2: %x\n\tr3: %x", r0, r1, r2, r3);
}

uint random() {
	static uint m_w, m_z;
    m_z = 36969 * (m_z & 65535) + (m_z >> 16);
    m_w = 18000 * (m_w & 65535) + (m_w >> 16);
    return (m_z << 16) + m_w;  /* 32-bit result */
}
