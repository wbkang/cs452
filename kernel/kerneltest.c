#include <test.h>
#include <util.h>
#include <memory.h>
#include <kernel.h>
#include <syscall.h>
#include <scheduler.h>
#include <nameserver.h>
#include <task.h>

volatile static int last_run_priority;
volatile static int last_run_tid = 0;

static void kerneltest_max_tasks_run() {
	int mytid = MyTid();
	int myparenttid = MyParentsTid();
	int mypriority = td_find(mytid)->priority;

	EXPECTMSG(1, myparenttid, "Wrong parent Tid");
	ASSERT(mytid > 0, "Wrong Tid");

	ASSERT(last_run_priority >= mypriority, "Priority violated. mypriority: %d, last_run_priority: %d", mypriority, last_run_priority);
	last_run_priority = mypriority;
}

static void kerneltest_max_tasks() {
	KTEST_START();
	TRACE("Testing Create with invalid priorities.");
	last_run_priority = MAX_PRIORITY;

	int tid = Create(-1, kerneltest_max_tasks_run);
	EXPECTMSG(-1, tid, "Should return -1 for an invalid priority");
	tid = Create(MAX_PRIORITY + 1, kerneltest_max_tasks_run);
	EXPECTMSG(-1, tid, "Should return -1 for an invalid priority");

	TRACE("Testing Create with invalid code.");
	tid = Create(1, NULL);
	EXPECTMSG(-3, tid, "Should return -3 for an invalid code");
	tid = Create(1, (func_t )0xdeadbeef);
	EXPECTMSG(-3, tid, "Should return -3 for an invalid code");

#if 0
	TRACE("Testing Create with maximum number of tasks.");
	for (int i = 0; i < TASK_LIST_SIZE - 2; i++) {
		tid = Create(i % (MAX_PRIORITY / 2), kerneltest_max_tasks_run);
		ASSERT(tid >= 0, "Task ID invalid %d", tid);
	}

	TRACE("Testing Create with no more task descriptor.");
	tid = Create(MIN_PRIORITY, kerneltest_max_tasks_run);
	EXPECTMSG(-2, tid, "Should return -2 when we used up ALL the task descriptors");
#endif
	KTEST_END();
}

static void kerneltest_exit() {
	KTEST_START();
	TRACE("Testing whether a task frees resource after running.");
	for (int i = 0; i < get_td_list_size() * 2; i++) {
		int tid = Create(MAX_PRIORITY, Exit);
		ASSERT(tid >= 0, "Task ID invalid %d", tid);
	}
	KTEST_END();
}

static void kerneltest_myparenttid() {
	KTEST_START();
	int mytid = MyTid();
	int myparenttid = MyParentsTid();
	int mypriority = td_find(mytid)->priority;
	if (last_run_tid != 1){
		EXPECTMSG(last_run_tid, myparenttid, "parent tid relationship wrong!");
	}
	last_run_tid = mytid;
	if (mypriority > MIN_PRIORITY) {
		ASSERT(Create(mypriority - 1, kerneltest_myparenttid) >= 0, "create task failed!?");
	}
	KTEST_END();
}


static void kerneltest_runner(int priority, func_t test) {
	kernel_init();
	last_run_priority = 0;
	last_run_tid = kernel_createtask(priority, test);
	kernel_run();
	mem_reset();
}

static void kerneltest_nameserver_overwrite() {
	KTEST_START();
	EXPECT(0, RegisterAs("aa"));
	EXPECT(MyTid(), WhoIs("aa"));
	KTEST_END();
}

static void kerneltest_nameserver() {
	KTEST_START();
	// invalid name
	EXPECT(NAMESERVER_ERROR_BADNAME, RegisterAs(""));
	EXPECT(NAMESERVER_ERROR_BADNAME, RegisterAs("o"));
	EXPECT(NAMESERVER_ERROR_BADNAME, RegisterAs("big"));
	EXPECT(NAMESERVER_ERROR_BADNAME, RegisterAs("this is clearly too long"));
	EXPECT(NAMESERVER_ERROR_BADNAME, WhoIs(""));
	EXPECT(NAMESERVER_ERROR_BADNAME, WhoIs("o"));
	EXPECT(NAMESERVER_ERROR_BADNAME, WhoIs("big"));
	EXPECT(NAMESERVER_ERROR_BADNAME, WhoIs("this is clearly too long"));

	// make sure this name was never registered
	EXPECT(NAMESERVER_ERROR_NOTREGISTERED, WhoIs("aa"));
	// try registering
	EXPECT(0, RegisterAs("aa"));
	EXPECT(MyTid(), WhoIs("aa"));
	// try overwriting the old entry
	EXPECT(0, RegisterAs("aa"));
	EXPECT(MyTid(), WhoIs("aa"));

	// try overwriting the old entry with another taks
	int overwritetid = Create(MAX_PRIORITY, kerneltest_nameserver_overwrite);
	ASSERT(overwritetid >= 0, "failed to create the tid overwriter");

	// make sure the tid overwriter worked.
	EXPECT(overwritetid, WhoIs("aa"));

	// try overwriting the old entry
	EXPECT(0, RegisterAs("aa"));
	EXPECT(MyTid(), WhoIs("aa"));
	KTEST_END();
}

static void kerneltest_nameserver_testallnames() {
	KTEST_START();
	// Exhaustive test testing every possible names.
	if (LONG_TEST_ENABLED) {
		char name[3];
		name[2] = '\0';
		for (char first = ASCII_PRINTABLE_START; first <= ASCII_PRINTABLE_END; first++) {
			for (char second = ASCII_PRINTABLE_START; second <= ASCII_PRINTABLE_END; second++) {
				name[0] = first;
				name[1] = second;
				EXPECT(-5, WhoIs(name));
				EXPECT(0, RegisterAs(name));
				EXPECT(MyTid(), WhoIs(name));
			}
		}
	}
	KTEST_END();
}

static void kerneltest_receive() {
	KTEST_START();
	int parent;
	int reply = 10000;
	int buf;

	for (int i = 0 ; i < 1000; i++) {
		EXPECT(sizeof buf, Receive(&parent, &buf, sizeof buf));
		EXPECT(MyParentsTid(), parent);
		EXPECT(reply - 10000, buf);
		EXPECT(0, Reply(parent, &reply, sizeof reply));
		reply++;
	}
	KTEST_END();
}

static void kerneltest_send() {
	KTEST_START();
	int num = 0;
	int reply = 0;
	int rcvtid = Create(MAX_PRIORITY - 2, kerneltest_receive);

	for(int i = 0; i < 1000; i++) {
		EXPECT(sizeof num, Send(rcvtid, &num, sizeof num, &reply, sizeof reply));
		EXPECT(num + 10000, reply);
		num++;
	}
	KTEST_END();
}

void kerneltest_run() {
	KTEST_START();
	mem_reset();
	kerneltest_runner(MAX_PRIORITY, kerneltest_max_tasks);
	kerneltest_runner(MAX_PRIORITY - 1, kerneltest_exit);
	kerneltest_runner(MAX_PRIORITY, kerneltest_myparenttid);
	kerneltest_runner(MAX_PRIORITY - 1, kerneltest_nameserver);
	kerneltest_runner(MAX_PRIORITY - 1, kerneltest_nameserver_testallnames);
	kerneltest_runner(MAX_PRIORITY - 1, kerneltest_send);
	mem_reset();
	KTEST_END();
}

