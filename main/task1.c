#include <task1.h>
#include <syscall.h>
#include <rpsserver.h>

void task1() {
	Create(0, rpsserver);
}
