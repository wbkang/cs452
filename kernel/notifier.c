#include <notifier.h>
#include <syscall.h>

typedef struct _tag_notifier_msg {
	int irq;
} notifier_args;

void notifier() {
	// init
	int tid;
	notifier_args args;
	Receive(&tid, &args, sizeof(args));
	Reply(tid, NULL, 0);
	// synchronize
	Send(tid, NULL, 0, NULL, 0);
	// serve
	for (;;) {
		AwaitEvent(args.irq);
		Send(tid, NULL, 0, NULL, 0);
	}
}

/*
 * API
 */

int notifier_new(int priority, int irq) {
	// create the notifier
	int tid = Create(priority, notifier);
	if (tid < 0) return tid;
	// pass in args
	notifier_args args;
	args.irq = irq;
	int rv = Send(tid, (void*) &args, sizeof(args), NULL, 0);
	if (rv < 0) return rv;
	return tid;
}
