#include <util.h>
#include <hardware.h>
#include <rawio.h>
#include <task.h>
#include <funcmap.h>
#include <console.h>

extern int _KERNEL_MEM_START;
extern int _KERNEL_MEM_END;

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
    static long a = 100001;
    a = (a * 125) % 2796203;
    return a;
}

void print_stack_trace(uint fp, int one) {
	if (!fp) return;

	__init_funclist();
	int pc = 0, lr = 0, depth = 20;

	bwprintf(1, CONSOLE_EFFECT(EFFECT_RESET));

	if (fp <= 0x218000 || fp >= 0x2000000) {
		bwprintf(1, "fp out of range: %x\n", fp);
		return;
	}

//	if (one) {
//		bwprintf(1, CONSOLE_EFFECT(EFFECT_BRIGHT) CONSOLE_EFFECT(EFFECT_FG_BLUE) "asmline\t\torigin\t\tfunction\n" CONSOLE_EFFECT(EFFECT_RESET));
//	}

	do {
		pc = VMEM(fp) - 16;
		int asm_line_num = (lr == 0) ? 0 : ((lr - pc) >> 2);
		if (one) {
			bwprintf(1, "%d\t\t%x\t%s\n", asm_line_num, pc, find_function_name(pc));
		} else {
			bwprintf(1, "%s @ %x+%d, ", find_function_name(pc), pc, asm_line_num);
		}

		if (lr == (int) Exit) break;

		lr = VMEM(fp - 4);
		fp = VMEM(fp - 12);
		if (fp < (int) &_KERNEL_MEM_START || (int) &_KERNEL_MEM_END <= fp) {
			break;
		} else if (depth-- < 0) {
			break;
		}
	} while (pc != REDBOOT_ENTRYPOINT && pc != (int) main);
	bwprintf(1, CONSOLE_EFFECT(EFFECT_RESET));
}
