#include <server/buffertask.h>
#include <queue.h>
#include <syscall.h>
#include <util.h>
#include <buffer.h>
#include <uconst.h>

// @TODO: temp
#include <console.h>
#include <ui/logdisplay.h>
#define DEBUGTASK 10

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
	logdisplay *log;
} buffertask_state;

static inline void tx(buffertask_state *state, int tid) {
	char item[state->item_size];
	buffer_get(state->items, item);

	if (MyTid() == DEBUGTASK) {
		logdisplay_printf(state->log,
			"%d get %d [%d]",
			tid,
			((msg_header*) item)->type,
			buffer_size(state->items)
		);
		logdisplay_flushline(state->log);
		// traincmd_receipt *rcpt = (traincmd_receipt*) item;
		// traincmd *cmd = &rcpt->cmd;
		// logdisplay_printf(state->log,
		// 	"%d get %d: switch %d to %c",
		// 	tid,
		// 	cmd->name,
		// 	cmd->arg1,
		// 	cmd->arg2
		// );
		// logdisplay_flushline(state->log);
	}

	Reply(tid, item, state->item_size);
}

static inline void handle_put(buffertask_state *state, int tid, void* item, int item_size) {
	ASSERT(!buffer_full(state->items), "buffer full");
	ASSERT(item_size <= state->item_size, "item size too big %d", item_size);
	Reply(tid, NULL, 0);
	buffer_put(state->items, item, item_size);
	if (!queue_empty(state->get_blocked)) {
		tx(state, (int) queue_get(state->get_blocked));
	}
}

static inline void handle_get(buffertask_state *state, int tid) {
	if (buffer_empty(state->items)) {
		ASSERT(!queue_full(state->get_blocked), "get_blocked buffer full, task %d is getting, task %d already blocked", tid, queue_peek(state->get_blocked));
		queue_put(state->get_blocked, (void*) tid);
	} else {
		tx(state, tid);
	}
}

void buffertask() {
	// init args
	int tid;
	int size = sizeof(buffertask_args) + sizeof(char) * SIZE_NAME;
	buffertask_args *args = malloc(size);
	Receive(&tid, args, size);
	Reply(tid, NULL, 0);

	// init name
	if (strlen(args->name) > 0) {
		RegisterAs(args->name);
	}

	// init state
	buffertask_state state;
	state.item_size = args->item_size;
	// @TODO: O M F G
	state.items = buffer_new(((STACK_SIZE / 2) / args->item_size), args->item_size);
	state.get_blocked = queue_new(NUM_BLOCKED);

	const int size_packet = max(sizeof(msg_header), state.item_size);
	void* packet = malloc(size_packet);

	if (MyTid() == DEBUGTASK) {
		state.log = logdisplay_new(console_new(COM2), 10, 56, 40, 100, ROUNDROBIN, "buffer log");
	}

	MEMCHECK();

	for (;;) {
		int size = Receive(&tid, packet, size_packet);
		ASSERT(size > 0, "bad packet");
		msg_header *header = (msg_header*) packet;
		switch (header->type) {
			case MSG_REQ:
				handle_get(&state, tid);
				break;
			default:
				if (MyTid() == DEBUGTASK) {
					if (header->type == 6) {
						traincmd_receipt *rcpt = packet;
						traincmd *cmd = &rcpt->cmd;
						if (cmd->name == 2) {
							logdisplay_printf(state.log,
								"%d put msg %d, %d: switch %d to %c [%d]",
								tid,
								header->type,
								cmd->name,
								cmd->arg1,
								cmd->arg2,
								buffer_size(state.items)
							);
							logdisplay_flushline(state.log);
						} else {
							logdisplay_printf(state.log,
								"%d put msg %d, %d(%d, %d) [%d]",
								tid,
								header->type,
								cmd->name,
								cmd->arg1,
								cmd->arg2,
								buffer_size(state.items)
							);
							logdisplay_flushline(state.log);
						}
					} else {
						logdisplay_printf(state.log,
							"### %d put %d ### [%d]",
							tid,
							header->type,
								buffer_size(state.items)
						);
						logdisplay_flushline(state.log);
					}
				}
				handle_put(&state, tid, packet, size);
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
	int namesize = 1 + (name ? strlen(name) : 0);
	ASSERT(namesize <= SIZE_NAME, "namesize exceeding SIZE_NAME");
	int size = sizeof(buffertask_args) + sizeof(char) * namesize;
	char mem[size];
	buffertask_args *args = (void*) mem;
	args->item_size = item_size;
	if (name) {
		strcpy(args->name, name);
	} else {
		args->name[0] = '\0';
	}
	int n = Send(tid, args, size, NULL, 0);
	if (n < 0) return n;
	return tid;
}

int buffertask_put(int tid, void* item, int item_size) {
	return Send(tid, item, item_size, NULL, 0);
}

int buffertask_get(int tid, void* item, int item_size) {
	msg_header msg;
	msg.type = MSG_REQ;
	return Send(tid, &msg, sizeof(msg), item, item_size);
}
