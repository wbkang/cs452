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
	bwprintf(COM2, "usermode syscall reqid:%d, args: %x\n", reqid, args);
	int retval = asm_syscall(reqid, args);
	bwprintf(COM2, "usermode syscall retval:%d\n", retval);
	return retval;
}

int Create(int priority, void(*code)()) {
	void* args[2] = { (void*)priority, (void*)(unsigned int)code };
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
	syscall(SYSCALL_EXIT, NULL);
}
