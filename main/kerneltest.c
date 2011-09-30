#include <test.h>
#include <util.h>
#include <memory.h>
#include <kernel.h>
#include <syscall.h>
#include <scheduler.h>

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
	for (int i = 0; i <TASK_LIST_SIZE-1; i++) {
		tid = Create(i % (MAX_PRIORITY / 2), kerneltest_max_tasks_run);
		ASSERT(tid >= 0, "Task ID invalid %d", tid);
	}

	TRACE("Testing Create with no more task descriptor.");
	tid = Create(MIN_PRIORITY, kerneltest_max_tasks_run);
	EXPECTMSG(-2, tid, "Should return -2 when we used up ALL the task descriptors");
}

static void kerneltest_exit() {
	TRACE("Testing whether a task frees resource after running.");
	for (int i = 0; i <TASK_LIST_SIZE * 2; i++) {
		int tid = Create(MAX_PRIORITY, Exit);
		ASSERT(tid >= 0, "Task ID invalid %d", tid);
	}
}

static void kerneltest_myparenttid() {
	TRACE("Testing parenttid relationship.");
	int mytid = MyTid();
	int myparenttid = MyParentsTid();
	int mypriority = td_find(mytid)->priority;

	if (last_run_tid != 0){
		EXPECTMSG(last_run_tid, myparenttid, "parent tid relationship wrong!");
	}

	last_run_tid = mytid;

	if (mypriority >= MIN_PRIORITY) {
		ASSERT(Create(mypriority-1, kerneltest_myparenttid) >= 0, "create task failed!?");
	}
}

static void kerneltest_runner_retiretd() {
	TRACE("Testing the retirement of lower tid from generational # exhaustion.");
	for (int i = 0; i < 0x8000; i++) {
		ASSERT(Create(MAX_PRIORITY, Exit) >= 0, "Create task failed");
	}

	int newtid = Create(MAX_PRIORITY, Exit);
	ASSERT(newtid == 2, "newtid expected to be 2, got %d (%x)", newtid, newtid);
}

static void kerneltest_runner(int priority, func_t test) {
	kernel_init();
	last_run_tid = 0;
	last_run_priority = 0;
	kernel_createtask(priority, test);
	kernel_runloop();
	mem_reset();
}


void kerneltest_run() {
	mem_reset();
	TRACE("######## kerneltest ########");

	kerneltest_runner(MAX_PRIORITY, kerneltest_max_tasks);
	kerneltest_runner(MAX_PRIORITY - 1, kerneltest_exit);
	kerneltest_runner(MAX_PRIORITY, kerneltest_myparenttid);
	kerneltest_runner(MAX_PRIORITY - 1, kerneltest_runner_retiretd);

	TRACE("######## kerneltest done ########");
	mem_reset();
}

