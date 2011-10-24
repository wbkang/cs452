#include <comnotifier.h>
#include <syscall.h>
#include <uconst.h>

typedef struct {
	int tid_target;
	int channel;
	int tid_com;
} comnotifier_args;

void comnotifier() {
	int tid;
	comnotifier_args args;
	Receive(&tid, &args, sizeof(args));

	msg_comin msg;
	msg.type = COM_IN;
	msg.channel = args.channel;

	ReplyNull(tid);

	for (;;) {
		msg.c = Getc(args.channel, args.tid_com);
		Send(args.tid_target, (void*) &msg, sizeof(msg), NULL, 0);
	}
}

int comnotifier_new(int priority, int tid_target, int channel, int tid_com) {
	comnotifier_args args;
	args.tid_target = tid_target;
	args.channel = channel;
	args.tid_com = tid_com;
	int tid = Create(priority, comnotifier);
	int rv = Send(tid, (void*) &args, sizeof(args), NULL, 0);
	if (rv < 0) return rv;
	return tid;
}
