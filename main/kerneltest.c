#include <test.h>
#include <util.h>
#include <memory.h>
#include <kernel.h>
#include <syscall.h>
#include <scheduler.h>
#include <nameserver.h>

volatile static int last_run_priority;
volatile static int last_run_tid = 0;

static void kerneltest_max_tasks_run() {
	int mytid = MyTid();
	int myparenttid = MyParentsTid();
	int mypriority = td_find(mytid)->priority;

	EXPECTMSG(0, myparenttid, "Wrong parent Tid");
	ASSERT(mytid > 0, "Wrong Tid");

	ASSERT(last_run_priority >= mypriority, "Priority violated. mypriority: %d, last_run_priority: %d", mypriority, last_run_priority);
	last_run_priority = mypriority;
}

static void kerneltest_max_tasks() {
	TRACE("Testing Create with invalid priorities.");
	last_run_priority = MAX_PRIORITY;

	int tid = Create(MIN_PRIORITY - 1, kerneltest_max_tasks_run);
	EXPECTMSG(-1, tid, "Should return -1 for an invalid priority");
	tid = Create(MAX_PRIORITY + 1, kerneltest_max_tasks_run);
	EXPECTMSG(-1, tid, "Should return -1 for an invalid priority");

	TRACE("Testing Create with invalid code.");
	tid = Create(1, NULL);
	EXPECTMSG(-3, tid, "Should return -3 for an invalid code");
	tid = Create(1, (func_t )0xdeadbeef);
	EXPECTMSG(-3, tid, "Should return -3 for an invalid code");

	TRACE("Testing Create with maximum number of tasks.");
	for (int i = 0; i < TASK_LIST_SIZE - 2; i++) {
		tid = Create(i % (MAX_PRIORITY / 2), kerneltest_max_tasks_run);
		ASSERT(tid >= 0, "Task ID invalid %d", tid);
	}

	TRACE("Testing Create with no more task descriptor.");
	tid = Create(MIN_PRIORITY, kerneltest_max_tasks_run);
	EXPECTMSG(-2, tid, "Should return -2 when we used up ALL the task descriptors");
}

static void kerneltest_exit() {
	TRACE("Testing whether a task frees resource after running.");
	for (int i = 0; i < TASK_LIST_SIZE * 2; i++) {
		int tid = Create(MAX_PRIORITY, Exit);
		ASSERT(tid >= 0, "Task ID invalid %d", tid);
	}
}

static void kerneltest_myparenttid() {
	int mytid = MyTid();
	int myparenttid = MyParentsTid();
	int mypriority = td_find(mytid)->priority;

	if (last_run_tid != 0){
		EXPECTMSG(last_run_tid, myparenttid, "parent tid relationship wrong!");
	}

	last_run_tid = mytid;

	if (mypriority > MIN_PRIORITY) {
		ASSERT(Create(mypriority - 1, kerneltest_myparenttid) >= 0, "create task failed!?");
	}
}


static void kerneltest_runner(int priority, func_t test) {
	kernel_init();
	last_run_tid = 0;
	last_run_priority = 0;
	kernel_createtask(priority, test);
	kernel_runloop();
	mem_reset();
}

static void kerneltest_nameserver_overwrite() {
	EXPECT(0, RegisterAs("aa"));
	EXPECT(MyTid(), WhoIs("aa"));
}

static void kerneltest_nameserver() {
	TEST_START();
	// invalid name
	EXPECT(-4, RegisterAs(""));
	EXPECT(-4, RegisterAs("o"));
	EXPECT(-4, RegisterAs("big"));
	EXPECT(-4, RegisterAs("this is clearly too long"));
	EXPECT(-4, WhoIs(""));
	EXPECT(-4, WhoIs("o"));
	EXPECT(-4, WhoIs("big"));
	EXPECT(-4, WhoIs("this is clearly too long"));

	// make sure this name was never registered
	EXPECT(-5, WhoIs("aa"));
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
	TEST_END();
}

static void kerneltest_nameserver_testallnames() {
	// Exhaustive test testing every possible names.
	if (LONG_TEST_ENABLED)
	{
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
}

void kerneltest_run() {
	TEST_START();
	mem_reset();
	kerneltest_runner(MAX_PRIORITY, kerneltest_max_tasks);
	kerneltest_runner(MAX_PRIORITY - 1, kerneltest_exit);
	kerneltest_runner(MAX_PRIORITY, kerneltest_myparenttid);
	kerneltest_runner(MAX_PRIORITY - 1, kerneltest_nameserver);
	kerneltest_runner(MAX_PRIORITY - 1, kerneltest_nameserver_testallnames);
	mem_reset();
	TEST_END();
}

