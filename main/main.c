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

static void receiver() {
	TRACE("my task id is: %d", MyTid());
	int tid;
	int msglen = 1024;
	char msg[msglen];
	char *reply = "yo, message received";
	int replylen = strlen(reply) + 1;
	TRACE("recieving...");
	Receive(&tid, msg, msglen);
	TRACE("got msg from task %d (%x)", tid, tid);
	TRACE("msg: %s (%x)", msg, msg);
	Reply(tid, reply, replylen);
}

static void driver() {
	/*int tid1 = Create(1, receiver);

	TRACE("created task with id %d", tid1);

	char *msg = "sender here, sending' stuff";
	int msglen = strlen(msg) + 1;
	int replylen = 1024;
	char reply[replylen];

	int len;

	len = Send(tid1, msg, msglen, reply, replylen);

	if (len < 0) {
		TRACE("ERROR: %d (%x)", len, len);
	} else {
		TRACE("reply: %s (%x)", reply, reply);
	}*/

	TRACE("my task id is: %d", MyTid());
	char *name = "X";
	int rv = RegisterAs(name);
	TRACE("RegisterAs returned %s (%x)", rv, rv);
	TRACE("WhoIs %s: %d", name, WhoIs(name));
}

int main(int argc, char *argv[]) {
	raw_init();
	test_run();
	kerneltest_run();
	kernel_init();
	kernel_createtask(0, receiver);
	kernel_createtask(0, driver);
	kernel_runloop();
	return 0;
}
