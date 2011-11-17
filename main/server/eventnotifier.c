#include <syscall.h>
#include <server/eventnotifier.h>

typedef struct _tag_notifier_msg {
	int eventid;
} notifier_args;

void eventnotifier() {
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

int eventnotifier_new(int priority, int eventid) {
	// create the notifier
	int tid = Create(priority, eventnotifier);
	if (tid < 0) return tid;
	// pass in args
	notifier_args args;
	args.eventid = eventid;
	int rv = Send(tid, &args, sizeof(args), NULL, 0);
	if (rv < 0) return rv;
	return tid;
}
