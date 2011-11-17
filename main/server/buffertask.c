#include <server/buffertask.h>
#include <queue.h>
#include <syscall.h>
#include <util.h>
#include <buffer.h>
#include <uconst.h>

#define NUM_BLOCKED 1
#define SIZE_NAME 4

typedef struct _tag_buffertask_args {
	int item_size;
	char name[];
} buffertask_args;

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

static inline void handle_put(buffertask_state *state, int tid, void* packet) {
	ASSERT(!buffer_full(state->items), "buffer full");
	void* item = ((msg_data *) packet)->data;
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

void buffertask() {
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
	state.items = buffer_new((STACK_SIZE - 1000) / args->item_size, args->item_size);
	state.get_blocked = queue_new(NUM_BLOCKED);

	const int size_req = sizeof(msg_req);
	const int size_data = sizeof(msg_data) + state.item_size;
	const int size_packet = MAX(size_req, size_data);
	void* packet = malloc(size_packet);

	for (;;) {
		int size = Receive(&tid, packet, size_packet);
		ASSERT(size > 0, "bad packet");
		msg_header *header = (msg_header*) packet;
		switch (header->type) {
			case REQ:
				handle_get(&state, tid);
				break;
			case DATA:
				handle_put(&state, tid, packet);
				break;
			default:
				ASSERT(0, "bad msg type: %d, tid: %d", header->type, tid);
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
	args->item_size = item_size;
	memcpy(args->name, name, namesize);
	int n = Send(tid, args, size, NULL, 0);
	if (n < 0) return n;
	return tid;
}

int buffertask_put(int tid, void* item, int item_size) {
	int size = sizeof(msg_req) + item_size;
	char mem[size];
	msg_data *msg = (void*) mem;
	msg->type = DATA;
	memcpy(msg->data, item, item_size);
	return Send(tid, msg, size, NULL, 0);
}

int buffertask_get(int tid, void* item, int item_size) {
	msg_req msg;
	msg.type = REQ;
	return Send(tid, &msg, sizeof(msg), item, item_size);
}
