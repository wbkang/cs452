#include <server/courier.h>
#include <syscall.h>
#include <uconst.h>

typedef struct _tag_courier_args {
	int tid_from;
	int tid_to;
} courier_args;

static void courier() {
	courier_args args;
	int tid;
	Receive(&tid, &args, sizeof(args));
	Reply(tid, NULL, 0);

	const int size_req = sizeof(msg_req);
	msg_req *req = malloc(size_req);
	req->type = REQ;

	const int size_packet = STACK_SIZE - 1000;
	void *packet = malloc(size_packet);

	for (;;) {
		int n = Send(args.tid_from, req, size_req, packet, size_packet);
		ASSERT(n >= 0, "bad send %d", n);
		Send(args.tid_to, packet, size_packet, NULL, 0);
	}
}

int courier_new(int priority, int tid_from, int tid_to) {
	int tid = Create(priority, courier);
	if (tid < 0) return tid;
	courier_args args = {tid_from, tid_to};
	int n = Send(tid, &args, sizeof(args), NULL, 0);
	if (n < 0) return n;
	return tid;
}
