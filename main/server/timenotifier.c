#include <syscall.h>
#include <uconst.h>
#include <server/timenotifier.h>

typedef struct _tag_timenotifier_msg {
	int tid_target;
	int ticks;
} timenotifier_args;

static void timenotifier() {
	// init
	int tid;
	timenotifier_args args;
	Receive(&tid, &args, sizeof(args));
	Reply(tid, NULL, 0);

	int tid_time = WhoIs(NAME_TIMESERVER);

	msg_time msg;
	msg.type = TIME;

	for (;;) {
		Delay(args.ticks, tid_time);
		msg.ticks = Time(tid_time);
		Send(args.tid_target, &msg, sizeof(msg), NULL, 0);
	}
}

/*
 * API
 */

int timenotifier_new(int tid_target, int priority, int ticks) {
	// create the notifier
	int tid = Create(priority, timenotifier);
	if (tid < 0) return tid;
	// pass in args
	timenotifier_args args;
	args.tid_target = tid_target;
	args.ticks = ticks;
	int rv = Send(tid, &args, sizeof(args), NULL, 0);
	if (rv < 0) return rv;
	return tid;
}
