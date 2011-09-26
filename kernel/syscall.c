#include <syscall.h>
#include <assembly.h>
#include <util.h>

/*
 *
 * Usermode implementation of the system calls.
 *
 */

static int syscall(int reqid, void** args) {
	return asm_syscall(reqid, args);
}

int Create(int priority, void(*code)()) {
	void** args = { priority, code };
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
	syscall(SYSCALL_PASS, NULL);
}
