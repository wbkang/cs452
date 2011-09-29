#include <syscall.h>
#include <assembly.h>
#include <util.h>

#include <hardware.h>
#include <rawio.h>
/*
 *
 * Usermode implementation of the system calls.
 *
 */

static int syscall(int reqid, void** args) {
	TRACE("usermode system call:\n\treqid: %d\n\targs: %x\n", reqid, args);

	int rv = 0xdeadbeef;

#ifndef __i386
	__asm(
			"mov r0, sp" "\n\t"
			"bl dump_registers(PLT)" "\n\t"
			"mov r0, %[reqid]" "\n\t"
			"mov r1, %[args]" "\n\t"
			"swi 0" "\n\t"
			"mov %[result], r0" "\n\t"
			: [result] "=r" (rv)
			: [reqid] "r" (reqid), [args] "r" (args)
			: "r0", "r1", "lr"
	);
#endif

	TRACE("\trv: %x\n", rv);
	return rv;
}

int Create(int priority, void(*code)()) {
	void* args[2] = { (void*) priority, (void*) (unsigned int) code };
	return syscall(SYSCALL_CREATE, args);
}

int MyTid() {
	return syscall(SYSCALL_MYTID, NULL);
}

int MyParentsTid() {
	return syscall(SYSCALL_MYPARENTTID, NULL);
}

void Pass() {
	syscall(SYSCALL_PASS, NULL);
}

void Exit() {
	TRACE("calling SYSCALL_EXIT\n");
	syscall(SYSCALL_EXIT, NULL);
}

void* malloc(uint size) {
	return (void*) syscall(SYSCALL_MALLOC, (void**) size);
}
