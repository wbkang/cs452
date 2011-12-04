#include <server/courier.h>
#include <syscall.h>
#include <uconst.h>
#include <string.h>

typedef struct _tag_courier_args {
	int tid_from;
	int tid_to;
	int item_size;
	char name[3];
} courier_args;

void courier() {
	courier_args args;
	int tid;
	Receive(&tid, &args, sizeof(args));
	if (strlen(args.name) > 0) {
		RegisterAs(args.name);
	}
	Reply(tid, NULL, 0);

	const int const size_req = sizeof(msg_header);
	msg_header *req = malloc(size_req);
	req->type = MSG_REQ;

	void* item = malloc(args.item_size);

	for (;;) {
		int size = Send(args.tid_from, req, size_req, item, args.item_size);
		ASSERT(size >= 0, "bad send %d", size);
		Send(args.tid_to, item, size, NULL, 0);
	}
}

int courier_new(int priority, int tid_from, int tid_to, int item_size, char *name) {
	int tid = Create(priority, courier);
	if (tid < 0) return tid;
	courier_args args;
	args.tid_from = tid_from;
	args.tid_to = tid_to;
	args.item_size = item_size;
	if (name) {
		strcpy(args.name, name);
	} else {
		args.name[0] = '\0';
	}
	int n = Send(tid, &args, sizeof(args), NULL, 0);
	if (n < 0) return n;
	return tid;
}
