#include <task1.h>
#include <syscall.h>
#include <rpsserver.h>
#include <rpsclient.h>

void task1() {
	Create(0, rpsserver);
	for (int i = 0; i < 4; i++) Create(0, rpsclient);
}
