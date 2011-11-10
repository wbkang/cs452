#include <queue.h>
#include <syscall.h>
#include <util.h>
#include <buffer.h>
#include <server/buffertask.h>

#define NUM_BLOCKED 1
#define SIZE_NAME 4

typedef struct _tag_buffertask_args {
	int size;
	int item_size;
	char name[];
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

static inline void tx(buffertask_state *state, int tid) {
	char item[state->item_size];
	buffer_get(state->items, item);
	Reply(tid, item, state->item_size);
}

static inline void handle_put(buffertask_state *state, int tid, void* item) {
	ASSERT(!buffer_full(state->items), "buffer full");
	buffer_put(state->items, item);
	Reply(tid, NULL, 0);
	if (!queue_empty(state->get_blocked)) {
		tx(state, (int) queue_get(state->get_blocked));
	}
}

static inline void handle_get(buffertask_state *state, int tid) {
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
	int size = sizeof(buffertask_args) + sizeof(char) * SIZE_NAME;
	char mem[size];
	buffertask_args *args = (void*) mem;
	Receive(&tid, args, size);
	Reply(tid, NULL, 0);

	// init name
	if (args->name) {
		RegisterAs(args->name);
	}

	// init state
	buffertask_state state;
	state.item_size = args->item_size;
	state.items = buffer_new(args->size, args->item_size);
	state.get_blocked = queue_new(NUM_BLOCKED);

	// init req
	int req_size = sizeof(buffertask_req) + state.item_size;
	buffertask_req *req = malloc(req_size);

	// serve
	for (;;) {
		Receive(&tid, req, req_size);
		switch (req->no) {
			case PUT:
				handle_put(&state, tid, req->item);
				break;
			case GET:
				handle_get(&state, tid);
				break;
			default:
				ERROR("bad req no: %d", req->no);
				break;
		}
	}
}

/*
 * API
 */

int buffertask_new(char *name, int priority, int item_size) {
	int tid = Create(priority, buffertask);
	if (tid < 0) return tid;
	int namesize = strlen(name) + 1;
	int size = sizeof(buffertask_args) + sizeof(char) * namesize;
	char mem[size];
	buffertask_args *args = (void*) mem;
	args->size = (STACK_SIZE - 1000) / item_size; // guess free memory
	args->item_size = item_size;
	memcpy(args->name, name, namesize);
	int n = Send(tid, args, size, NULL, 0);
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
