#include <rawio.h>
#include <kernel.h>
#include <test.h>
#include <syscall.h>
#include <string.h>
#include <kerneltest.h>

static void task2() {
	bwprintf(COM2, "task id: %d, parent's task id: %d\n", MyTid(), MyParentsTid());
	Pass();
	bwprintf(COM2, "task id: %d, parent's task id: %d\n", MyTid(), MyParentsTid());
	TRACE("Exiting %d", MyTid());
}

static void task1() {
	for (int i = 0; i < 4; i++) {
		int priority = 2 * (i >> 1);
		bwprintf(COM2, "Created: %d\n", Create(priority, task2));
	}
	bwprintf(COM2, "First: exiting\n");
}

static void task3() {
	char *msg = "this is a message";
	int msglen = strlen(msg);
	char *reply = "this is a reply message";
	int replylen = strlen(reply);
	bwprintf(COM2, "Send: ", Send(0xface, msg, msglen, reply, replylen));
}

static void task4() {
	char *msg = "this is a message";
	int msglen = strlen(msg);
	bwprintf(COM2, "Recieve: ", Recieve((int *) 0xface, msg, msglen));
}

static void task5() {
	char *reply = "this is a reply message";
	int replylen = strlen(reply);
	bwprintf(COM2, "Send: ", Reply(0xface, reply, replylen));
}

int main(int argc, char *argv[]) {
	raw_init();
	char *test = "this is a big long test string";
	TRACE("%d", strlen(test));
	test_run();
	kerneltest_run();
	kernel_init();
	kernel_createtask(1, task3);
	kernel_createtask(1, task4);
	kernel_createtask(1, task5);
	kernel_runloop();
	return 0;
}
