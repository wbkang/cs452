#include <server/publisher.h>
#include <syscall.h>
#include <server/courier.h>
#include <server/buffertask.h>
#include <uconst.h>
#include <string.h>

// @TODO: temp
#include <console.h>
#include <ui/logdisplay.h>

#define NUM_SUBSCRIBERS 128

typedef struct {
	int item_size;
	int priority;
	int num_clients;
	int tid_to[NUM_SUBSCRIBERS];
} publisher_state;

typedef struct {
	int item_size;
	int priority;
} publisher_args;

static void handle_pub(publisher_state *state, void* packet, int size) {
	for (int i = 0; i < state->num_clients; i++) {
		Send(state->tid_to[i], packet, size, NULL, 0);
	}
}

static void handle_sub(publisher_state *state, void* packet) {
	ASSERT(state->num_clients + 1 < NUM_SUBSCRIBERS, "too many subscribers");

	msg_sub *msg = packet;
	int tid = msg->tid;

	int tid_buffer = buffertask_new(NULL, state->priority, state->item_size);
	ASSERT(tid_buffer >= 0, "unable to create a new buffer task");
	int tid_courier = courier_new(state->priority, tid_buffer, tid, state->item_size, NULL);
	ASSERT(tid_courier >= 0, "unable to create a new courier");

	state->tid_to[state->num_clients++] = tid_buffer;
}

void publisher() {
	publisher_args args;
	int tid;
	Receive(&tid, &args, sizeof(args));
	Reply(tid, NULL, 0);

	publisher_state state;
	state.item_size = args.item_size;
	state.priority = args.priority;
	state.num_clients = 0;

	const int const packet_size = max(sizeof(msg_header), state.item_size);
	void* packet = malloc(packet_size);

	for (;;) {
		int tid;
		int size = Receive(&tid, packet, packet_size);
		Reply(tid, NULL, 0);
		ASSERT(size >= sizeof(msg_header), "bad packet");
		msg_header *header = packet;
		switch (header->type) {
			case MSG_SUB:
				handle_sub(&state, packet);
				break;
			case MSG_REQ:
				ASSERT(0, "this is something I cant pass along");
				break;
			default:
				handle_pub(&state, packet, size);
				break;
		}
	}
}

int publisher_new(char *name, int priority, int item_size) {
	int tid_buffer = buffertask_new(name, priority, item_size);
	if (tid_buffer < 0) return tid_buffer;
	int tid_publisher = Create(priority, publisher);
	if (tid_publisher < 0) return tid_publisher;
	publisher_args args;
	args.item_size = item_size;
	args.priority = priority;
	int n = Send(tid_publisher, &args, sizeof(args), NULL, 0);
	if (n < 0) return n;
	int tid_courier = courier_new(priority, tid_buffer, tid_publisher, item_size, NULL);
	if (tid_courier < 0) return tid_courier;
	return tid_buffer;
}

int publisher_pub(int tid, void* item, int item_size) {
	ASSERT(tid >= 0, "invalid publisher");
	return Send(tid, item, item_size, NULL, 0);
}

int publisher_sub(int tid_publisher, int tid) {
	ASSERT(tid_publisher >= 0, "invalid publisher");
	ASSERT(tid >= 0, "bad tid");
	msg_sub packet;
	packet.type = MSG_SUB;
	packet.tid = tid;
	return Send(tid_publisher, &packet, sizeof(msg_sub), NULL, 0);
}
