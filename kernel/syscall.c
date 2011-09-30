#include <syscall.h>
#include <assembly.h>
/*
 *
 * Usermode implementation of the system calls.
 *
 */

static int syscall(int reqid, int a1, int a2, int *args) {
	// TRACE("usermode system call:\n\treqid: %d\n\targs: %x", reqid, args);

	int rv = 0xdeadbeef;

#ifndef __i386
	__asm(
			"mov r0, %[reqid]" "\n\t"
			"mov r1, %[a1]" "\n\t"
			"mov r2, %[a2]" "\n\t"
			"mov r3, %[args]" "\n\t"
			"swi 0" "\n\t"
			"mov %[result], r0" "\n\t"
			: [result] "=r" (rv)
			: [reqid] "r" (reqid), [a1] "r" (a1), [a2] "r" (a2), [args] "r" (args)
			: "r0", "r1", "r2", "r3"
	);
#endif

	// TRACE("\trv: %x", rv);
	return rv;
}

int Create(int priority, func_t code) {
	return syscall(SYSCALL_CREATE, priority, (int) code, NULL);
}

int MyTid() {
	return syscall(SYSCALL_MYTID, NULL, NULL, NULL);
}

int MyParentsTid() {
	return syscall(SYSCALL_MYPARENTTID, NULL, NULL, NULL);
}

void Pass() {
	syscall(SYSCALL_PASS, NULL, NULL, NULL);
}

void Exit() {
	syscall(SYSCALL_EXIT, NULL, NULL, NULL);
}

void* malloc(uint size) {
	return (void*) syscall(SYSCALL_MALLOC, (int) size, NULL, NULL);
}
