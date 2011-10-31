#include <buffertask.h>

#define NUM_BLOCKED 1

typedef struct _tag_buffertask_args {
	int size;
	int item_size;
} buffertask_args;

typedef struct _tag_buffertask_req {
	enum { PUT, GET } no;
	char item[];
} buffertask_req;

typedef struct _tag_buffertask_state {
	int item_size;
	buffer *items;
	queue *get_blocked;
} buffertask_state;

static inline void tx(buffertask_state* state, int tid) {
	char item[state->item_size];
	buffer_get(state->items, item);
	Reply(tid, item, state->item_size);
}

static inline void handle_put(buffertask_state* state, int tid, void* item) {
	ASSERT(!buffer_full(state->items), "buffer full");
	buffer_put(state->items, item);
	Reply(tid, Null, 0);
	if (!queue_empty(state->get_blocked)) {
		tx(state, (int) queue_get(state->get_blocked));
	}
}

static inline void handle_get(buffertask_state* state, int tid) {
	if (buffer_empty(state->items)) {
		ASSERT(!queue_full(state->get_blocked), "buffer full");
		queue_put(state->get_blocked, (void*) tid);
	} else {
		tx(state, tid);
	}
}

static void buffertask() {
	// init args
	int tid;
	buffertask_args args;
	Receive(&tid, args, sizeof(args));
	Reply(tid, NULL, 0);

	// init state
	buffertask_state state;
	state.item_size = args.item_size;
	state.items = buffer_new(args.size, args.item_size);
	state.get_blocked = queue_new(NUM_BLOCKED);

	// init req
	int req_size = sizeof(buffertask_req) + state.item_size;
	buffertask_req *req = malloc(req_size);

	// serve
	for (;;) {
		Receive(&tid, &req, req_size);
		switch (req->no) {
			case PUT:
				handle_put(&state, tid, req->item);
				break;
			case GET:
				handle_get(&state, tid);
				break;
			default:
				ERROR("bad req no: %d", req.no);
				break;
		}
	}
}

/*
 * API
 */

int buffertask_new(int priority, int item_size) {
	int tid = Create(priority, buffertask);
	if (tid < 0) return tid;
	buffertask_args args;
	args.size = 1024; // (65536 - x) / item_size
	args.item_size = item_size;
	int n = Send(tid, &args, sizeof(args), NULL, 0);
	if (n < 0) return n;
	return tid;
}

int buffertask_put(int tid, void* item, int item_size) {
	int size = sizeof(buffertask_req) + item_size;
	char mem[size];
	buffertask_req *req = (void*) mem;
	req->no = PUT;
	memcpy(req->item, item, item_size);
	return Send(tid, req, size, NULL, 0);
}

int buffertask_get(int tid, void* item, int item_size) {
	buffertask_req req;
	req.no = GET;
	return Send(tid, &req, sizeof(req), item, item_size);
}
