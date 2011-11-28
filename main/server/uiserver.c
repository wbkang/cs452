#include <server/uiserver.h>
#include <ui/logdisplay.h>
#include <ui/a0_track_template.h>
#include <console.h>
#include <util.h>
#include <string.h>

#define CONSOLE_DUMP_LINE (CONSOLE_CMD_LINE + 4)
#define CONSOLE_DUMP_COL 1

typedef struct uiserver_state uiserver_state;
struct uiserver_state {
	console *con;
	logdisplay *logs[NUM_LOGS];
};

static void handle_log(uiserver_state *state, void* msg);

void uiserver() {
	uiserver_state state;
	state.con = console_new(COM2);
	state.logs[UI_A0LOG] = logdisplay_new(state.con, CONSOLE_DUMP_LINE, CONSOLE_DUMP_COL, 19, 70, ROUNDROBIN, "a0log");
	state.logs[UI_TRAINLOCLOG] = logdisplay_new(state.con, 11, 56, 8, 100, ROUNDROBIN, "train location log");
	state.logs[UI_TRAINATTRLOG] = logdisplay_new(state.con, 11 + 9, 56, 8, 100, ROUNDROBIN, "location attribution log");
	state.logs[UI_ENG_TRIPLOG] = logdisplay_new(state.con, 11 + 20, 56 + 15, 8 + 12, 100, ROUNDROBIN, "engineer triplog");

	const int size_packet = min(STACK_SIZE / 2, MAX_MSG_SIZE);
	void* packet = malloc(size_packet);

	for (;;) {
		int tid;
		int size = Receive(&tid, packet, size_packet);
		Reply(tid, NULL, 0);
		ASSERT(size > 0, "bad packet");
		msg_header *header = (msg_header*) packet;
		switch (header->type) {
			case MSG_UI:
				handle_log(&state, packet);
				break;
			default:
				ASSERT(0, "ui server received a bad header type %d from tid:%d" , header->type, tid);
				break;
		}
	}
}

static void handle_log(uiserver_state *state, void* msg) {
	msg_ui *uimsg = msg;

	switch (uimsg->uitype) {
		case UIMSG_LOG:
			logdisplay_puts(state->logs[uimsg->id], uimsg->str);
			logdisplay_flushline(state->logs[uimsg->id]);
			break;
		default:
			ASSERT(0, "ui server received a bad msg type %d" , uimsg->uitype);
			break;
	}
}

void uiserver_log(int logid, char *str, int tid_ui) {
	// TODO this is kindf of a lazy hack
	char buf[MAX_LOG_COL + sizeof(msg_ui) + 1];
	msg_ui *uimsg = (msg_ui*) buf;
	uimsg->type = MSG_UI;
	uimsg->uitype = UIMSG_LOG;
	uimsg->strlen = strlen(str);
	uimsg->id = logid;
	strcpy(uimsg->str, str);
	int rv = Send(tid_ui, uimsg, sizeof(msg_ui) + uimsg->strlen + 1, NULL, 0);
	ASSERT(rv >= 0, "send failed. rv:%d, logid:%d, str:%s, tid_ui:%d",
			rv, logid, str, tid_ui);
}
