#include <syscall.h>
#include <server/notifier.h>

typedef struct _tag_notifier_msg {
	int eventid;
} notifier_args;

static void notifier() {
	// init
	int tid;
	notifier_args args;
	Receive(&tid, &args, sizeof(args));
	Reply(tid, NULL, 0);
	// synchronize
	Send(tid, NULL, 0, NULL, 0);
	// serve
	for (;;) {
		AwaitEvent(args.eventid);
		Send(tid, NULL, 0, NULL, 0);
	}
}

/*
 * API
 */

int notifier_new(int priority, int eventid) {
	// create the notifier
	int tid = Create(priority, notifier);
	if (tid < 0) return tid;
	// pass in args
	notifier_args args;
	args.eventid = eventid;
	int rv = Send(tid, &args, sizeof(args), NULL, 0);
	if (rv < 0) return rv;
	return tid;
}
