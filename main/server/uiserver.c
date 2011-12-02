#include <server/uiserver.h>
#include <server/timenotifier.h>
#include <server/courier.h>
#include <server/buffertask.h>
#include <ui/logdisplay.h>
#include <ui/a0_track_template.h>
#include <console.h>
#include <util.h>
#include <string.h>

#define CONSOLE_DUMP_LINE (CONSOLE_CMD_LINE + 4)
#define CONSOLE_DUMP_COL 1

#define MAX_UICLIENTS 300

#define SCREEN_WIDTH 100
#define SCREEN_HEIGHT 40

#define SCREEN_TO_LOC(line, col) ((line - 1) * SCREEN_WIDTH + (col - 1))
#define SCREEN_LOC_LINE(loc) ((loc / SCREEN_WIDTH))
#define SCREEN_LOC_COL(loc) ((loc % SCREEN_WIDTH) + 1)

typedef struct ui_context {
	char effectflag;
	char color;
	int line;
	int col;
} ui_context;

typedef struct ui_cellinfo {
	char effectflag;
	char color;
	char c;
	char changed;
} ui_cellinfo;

typedef struct uiserver_state uiserver_state;
struct uiserver_state {
	console *con;
	int client_count;
	ui_context *context[MAX_UICLIENTS];
	ui_cellinfo screen[SCREEN_HEIGHT * SCREEN_WIDTH];
};

static void handle_uimsg(uiserver_state *state, void* msg);
static void handle_uiout(uiserver_state *state, msg_ui *uimsg);
static void handle_refresh(uiserver_state *state);

void uiserver() {
	RegisterAs(NAME_UISERVER_S);
	uiserver_state state;
	state.con = console_new(COM2);
	int tid_time = timenotifier_new(MyTid(), PRIORITY_UISERVER, MS2TICK(100)); // 60fps
	ASSERT(tid_time >= 0, "failed to create timenotifier");
	state.client_count = 0;

	for (int i = 0; i < SCREEN_HEIGHT * SCREEN_WIDTH; i++) {
		state.screen[i].c = 0;
		state.screen[i].color = 0;
		state.screen[i].effectflag = 0;
		state.screen[i].changed = FALSE;
	}

	const int size_packet = min(STACK_SIZE / 2, MAX_MSG_SIZE);
	void* packet = malloc(size_packet);

	for (;;) {
		int tid;
		int size = Receive(&tid, packet, size_packet);
		ASSERT(size > 0, "bad packet");
		int rv = Reply(tid, NULL, 0);
		ASSERT(rv >= 0, "reply failed");
		msg_header *header = (msg_header*) packet;
		switch (header->type) {
			case MSG_UI:
				handle_uimsg(&state, packet);
				break;
			case MSG_TIME:
				handle_refresh(&state);
				break;
			default:
				ASSERT(0, "ui server received a bad header type %d from tid:%d" , header->type, tid);
				break;
		}
	}
}

static void handle_cellout(uiserver_state *state, ui_cellinfo *lastoutcell, ui_cellinfo *cell) {
	int diffeffect = lastoutcell == NULL
			|| cell->color != lastoutcell->color
			|| cell->effectflag != lastoutcell->effectflag;
	int diffcontent = lastoutcell == NULL
			|| cell->c != lastoutcell->c;

	if (diffcontent || diffeffect) {
		int cellidx = cell - state->screen;
		int relocate = SCREEN_LOC_COL(cellidx) == 1
				|| lastoutcell == NULL
				|| (cell - lastoutcell) != 1;

		if (diffeffect) {
			console_effect_reset(state->con);

			if (cell->effectflag & UIEFFECT_BRIGHT) {
				console_effect(state->con, EFFECT_BRIGHT);
			}
			if (cell->effectflag & UIEFFECT_UNDERSCORE) {
				console_effect(state->con, EFFECT_UNDERSCORE);
			}
			if (cell->effectflag & UIEFFECT_FGCOLOR) {
				console_effect(state->con, cell->color);
			}
		}

		if (relocate) {
			console_move(state->con, SCREEN_LOC_LINE(cellidx), SCREEN_LOC_COL(cellidx));
		}
//		bwprintf(1, "%c", cell->c);
		console_printf(state->con, "%c", cell->c);
//		console_flush(state->con);

		cell->changed = FALSE;
	}
}

static void handle_refresh(uiserver_state *state) {
	ui_cellinfo *lastoutcell = NULL;
	console_cursor_save(state->con);
	for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
		ui_cellinfo *cell = &state->screen[i];
		if (cell->changed) {
			handle_cellout(state, lastoutcell, cell);
			lastoutcell = cell;
		}
	}
	console_cursor_unsave(state->con);
	console_flush(state->con);
}

static void handle_uiout(uiserver_state *state, msg_ui *uimsg) {
	int id = uimsg->id;
	ui_context *ctx = state->context[id];
	int curloc = SCREEN_TO_LOC(ctx->line, ctx->col);

	for (int i = 0; i < uimsg->strlen; i++) {
		ui_cellinfo *cell = &state->screen[curloc++];
		cell->c = uimsg->str[i];
		cell->effectflag = ctx->effectflag;
		cell->color = ctx->color;
		cell->changed = TRUE;
	}
}

static void handle_uimsg(uiserver_state *state, void* msg) {
	msg_ui *uimsg = msg;
	switch (uimsg->uimsg) {
		case UIMSG_REGISTER: {
			int clienttid = uimsg->id;
			int id = state->client_count++;
			ASSERT(id < MAX_UICLIENTS, "tooooo many ui clients. id: %d", id);
			state->context[id] = malloc(sizeof(ui_context));
			state->context[id]->color = 0;
			state->context[id]->effectflag = 0;
			state->context[id]->line = 1;
			state->context[id]->col = 1;
			int rv = Send(clienttid, &id, sizeof(int), NULL, 0);
			ASSERT(rv >= 0 , "send failed");
			break;
		}
		case UIMSG_EFFECT: {
			int id = uimsg->id;
			state->context[id]->effectflag = uimsg->flag;
			state->context[id]->color = uimsg->fgcolor;
			break;
		}
		case UIMSG_MOVE: {
			int id = uimsg->id;
			state->context[id]->line = uimsg->line;
			state->context[id]->col = uimsg->col;
			break;
		}
		case UIMSG_OUT: {
			handle_uiout(state, uimsg);
			break;
		}
		default:
			ASSERT(0, "unknown uimsg type: %d", uimsg->uimsg);
			return; // unreachable
	}
}

int uiserver_new() {
	const int msgsize = MAX_MSG_SIZE / 10; // TODO sketchy
	int tid_server = Create(PRIORITY_UISERVER, uiserver);
	ASSERT(tid_server >= 0, "failed to create ui server");
	int tid_buf = buffertask_new(NAME_UISERVER, PRIORITY_UISERVER, msgsize);
	ASSERT(tid_buf >= 0, "failed to create ui buffer");
	courier_new(PRIORITY_UISERVER, tid_buf, tid_server, msgsize, NAME_UISERVER_C);
	return tid_buf;
}

int uiserver_register(int tid) {
	msg_ui uimsg;
	uimsg.type = MSG_UI;
	uimsg.uimsg = UIMSG_REGISTER;
	uimsg.id = MyTid();
	int rv = Send(tid, &uimsg, sizeof(uimsg), NULL, 0);
	ASSERT(rv >= 0, "send failed");
	int tid_recv;
	int id;
	rv = Receive(&tid_recv, &id, sizeof(id));
	ASSERT(rv >= 0, "receive failed");
	rv = Reply(tid_recv, NULL, 0);
	ASSERT(rv >= 0, "reply failed");
	return id;
}


void uiserver_effect(int tid, int id, int flag, int color) {
	msg_ui uimsg;
	uimsg.type = MSG_UI;
	uimsg.uimsg = UIMSG_EFFECT;
	uimsg.id = id;
	uimsg.fgcolor = flag;
	uimsg.fgcolor = color;

	int rv = Send(tid, &uimsg, sizeof(uimsg), NULL, 0);
	ASSERT(rv >= 0, "send failed");
}

void uiserver_out(int tid, int id, char *out) {
	int len = strlen(out);
	char msg[sizeof(msg_ui) + len + 1];
	msg_ui *uimsg = (msg_ui*) msg;
	uimsg->type = MSG_UI;
	uimsg->uimsg = UIMSG_OUT;
	uimsg->id = id;
	strcpy(uimsg->str, out);
	uimsg->strlen = len;

	int rv = Send(tid, uimsg, sizeof(msg), NULL, 0);
	ASSERT(rv >= 0, "send failed");
}

void uiserver_move(int tid, int id, int line, int col) {
	msg_ui uimsg;
	uimsg.type = MSG_UI;
	uimsg.uimsg = UIMSG_MOVE;
	uimsg.id = id;
	uimsg.line = line;
	uimsg.col = col;

	int rv = Send(tid, &uimsg, sizeof(uimsg), NULL, 0);
	ASSERT(rv >= 0, "send failed");
}
