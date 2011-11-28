#include <server/publisher.h>
#include <syscall.h>
#include <server/courier.h>
#include <server/buffertask.h>
#include <uconst.h>
#include <string.h>

#define NUM_SUBSCRIBERS 32

typedef struct {
	int num_clients;
	int tid_to[NUM_SUBSCRIBERS];
} publisher_state;

static void handle_pub(publisher_state *state, void* packet, int size) {
	for (int i = 0; i < state->num_clients; i++) {
		Send(state->tid_to[i], packet, size, NULL, 0);
	}
}

static void handle_sub(publisher_state *state, void* packet) {
	ASSERT(state->num_clients + 1 < NUM_SUBSCRIBERS, "too many subscribers");
	msg_sub *msg = (msg_sub*) packet;
	state->tid_to[state->num_clients++] = msg->tid;
}

void publisher() {
	publisher_state state;
	state.num_clients = 0;

	const int size_packet = min(STACK_SIZE - 1000, MAX_MSG_SIZE);
	void* packet = malloc(size_packet);

	for (;;) {
		int tid;
		int size = Receive(&tid, packet, size_packet);
		Reply(tid, NULL, 0);
		ASSERT(size > 0, "bad packet");
		msg_header *header = (msg_header*) packet;
		switch (header->type) {
			case MSG_SUB:
				handle_sub(&state, packet);
				break;
			default:
				handle_pub(&state, packet, size);
				break;
		}
	}
}

int publisher_new(char name[], int priority, int item_size) {
	int tid_buffer = buffertask_new(name, priority, item_size);
	if (tid_buffer < 0) return tid_buffer;
	int tid_publisher = Create(priority, publisher);
	if (tid_publisher < 0) return tid_publisher;
	int tid_courier = courier_new(priority, tid_buffer, tid_publisher);
	if (tid_courier < 0) return tid_courier;
	return tid_buffer;
}

int publisher_pub(int tid, void* item, int item_size) {
	return Send(tid, item, item_size, NULL, 0);
}

int publisher_sub(int tid_publisher, int tid) {
	msg_sub packet;
	packet.type = MSG_SUB;
	packet.tid = tid;
	return Send(tid_publisher, &packet, sizeof(packet), NULL, 0);
}
