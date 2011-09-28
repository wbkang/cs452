#include <util.h>
#include <hardware.h>
#include <rawio.h>
#include <task.h>

void die()
{
	//stub
	while(1);
}

void errormsg(char* msg)
{
	bwprintf(COM2, msg);
}

void dump_registers(int r0, int r1, int r2, int r3) {
	bwprintf(COM2, "registers 1:%x\t2:%x\t3:%x\t4:%x\t\n", r0, r1, r2, r3);
}
