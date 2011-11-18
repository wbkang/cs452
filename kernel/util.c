#include <util.h>
#include <hardware.h>
#include <rawio.h>
#include <task.h>
#include <funcmap.h>
#include <console.h>

extern int _KERNEL_MEM_START;
extern int _KERNEL_MEM_END;

static int HERPDERP;

int *getherp() {
	return &HERPDERP;
}

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

static inline char* find_function_name(uint pc) {
	funcinfo* fl = __getfunclist();
	int i = 0;

	while (fl[i].fn != 0) {
		if (fl[i].fn == pc) return fl[i].name;
		i++;
	}

	return "[unknown function]";
}

void print_stack_trace(uint fp, int clearscreen) {
	if (!fp) return;

	__init_funclist();
	int pc = 0, lr = 0, depth = 20;

	if (clearscreen) {
		bwprintf(1, CONSOLE_CLEAR "\x1B[1;1H");
	}

	bwprintf(1, CONSOLE_EFFECT(EFFECT_RESET) CONSOLE_EFFECT(EFFECT_BRIGHT) "------stack trace----\n");
	bwprintf(1, CONSOLE_EFFECT(EFFECT_FG_BLUE) "asmline\t\torigin\t\tfunction\n" CONSOLE_EFFECT(EFFECT_RESET));
	do {
		pc = VMEM(fp) - 16;
		int asm_line_num = (lr == 0) ? 0 : ((lr - pc) >> 2);
		bwprintf(1, "%d\t\t%x\t%s\n", asm_line_num, pc, find_function_name(pc));

		if (lr == (int) Exit) break;

		lr = VMEM(fp - 4);
		fp = VMEM(fp - 12);
		if (fp < (int) &_KERNEL_MEM_START || (int) &_KERNEL_MEM_END <= fp) {
//			bwprintf(1, "next fp out of range:%x\n", fp);
			break;
		} else if (depth-- < 0) {
//			bwprintf(1, "stack trace too deep\n", fp);
			break;
		}
	} while (pc != REDBOOT_ENTRYPOINT && pc != (int) main);
	bwprintf(1, CONSOLE_EFFECT(EFFECT_RESET));
}
