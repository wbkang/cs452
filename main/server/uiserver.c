#include <server/uiserver.h>
#include <server/timenotifier.h>
#include <server/courier.h>
#include <server/buffertask.h>
#include <ui/logdisplay.h>
#include <ui/a0_track_template.h>
#include <console.h>
#include <util.h>
#include <string.h>

#define SCREEN_WIDTH 140
#define SCREEN_HEIGHT 50

#define MAX_UICLIENTS 300
#define MAX_UIMSGSIZE (sizeof(msg_ui) + SCREEN_WIDTH + 1)

#define SCREEN_TO_LOC(line, col) ((line - 1) * SCREEN_WIDTH + (col - 1))
#define SCREEN_LOC_LINE(loc) ((loc / SCREEN_WIDTH) + 1)
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
	char changed;
};

static void handle_uimsg(uiserver_state *state, void* msg);
static void handle_uiout(uiserver_state *state, msg_ui *uimsg);
static void handle_refresh(uiserver_state *state);
static void handle_force_refresh(uiserver_state *state);

void uiserver() {
	RegisterAs(NAME_UISERVER_S);
	uiserver_state state;
	state.con = console_new(COM2);
	int tid_time = timenotifier_new(MyTid(), PRIORITY_UISERVER, MS2TICK(17)); // 60fps
	ASSERT(tid_time >= 0, "failed to create timenotifier");
	state.client_count = 0;
	state.changed = FALSE;

	for (int i = 0; i < SCREEN_HEIGHT * SCREEN_WIDTH; i++) {
		state.screen[i].c = ' ';
		state.screen[i].color = 0;
		state.screen[i].effectflag = 0;
		state.screen[i].changed = FALSE;
	}

	const int size_packet = MAX_MSG_SIZE / 2;
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
	int cellidx = cell - state->screen;
	int relocate = SCREEN_LOC_COL(cellidx) == 1
			|| lastoutcell == NULL
			|| (cell - lastoutcell) != 1;

	if (relocate) {
		console_flush(state->con);
		console_flushcom(state->con);
		console_move(state->con, SCREEN_LOC_LINE(cellidx), SCREEN_LOC_COL(cellidx));
	}

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

	console_printf(state->con, "%c", cell->c);

	cell->changed = FALSE;
}

static void handle_refresh(uiserver_state *state) {
	ui_cellinfo *lastoutcell = NULL;
	for (int i = 0; state->changed && i <  SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
		ui_cellinfo *cell = &state->screen[i];
		if (cell->changed) {
			handle_cellout(state, lastoutcell, cell);
			lastoutcell = cell;
		}
	}
	state->changed = FALSE;
	console_flush(state->con);
	console_flushcom(state->con);
}

static void handle_uiout(uiserver_state *state, msg_ui *uimsg) {
	int id = uimsg->id;
	ui_context *ctx = state->context[id];
	int curloc = SCREEN_TO_LOC(ctx->line, ctx->col);

	for (int i = 0; i < uimsg->strlen && curloc < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
		if (SCREEN_LOC_LINE(curloc) != ctx->line) {
			break; // linewrap disabled
		}
		ui_cellinfo *cell = &state->screen[curloc++];
		char newchar = uimsg->str[i];
		char neweffect = ctx->effectflag;
		char newcolor = ctx->color;
		if (newchar == '\t') {
			ctx->col = min(ALIGN(ctx->col, 8), SCREEN_WIDTH + 1);
			continue;
		}
		cell->changed |= (cell->c != newchar) || (cell->effectflag != neweffect) || (cell->color != newcolor);
		state->changed |= cell->changed;
		cell->c = newchar;
		cell->effectflag = neweffect;
		cell->color = newcolor;
	}

	ctx->line = SCREEN_LOC_LINE(curloc);
	ctx->col = SCREEN_LOC_COL(curloc);
}

static void handle_force_refresh(uiserver_state *state) {
	console_clear(state->con);
	for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
		ui_cellinfo *cell = &state->screen[i];
		cell->changed = TRUE;
	}
	state->changed = TRUE;
}

static void handle_movecursor(uiserver_state *this, int line, int col) {
	console_flush(this->con);
	console_cursor_unsave(this->con);
	console_move(this->con, line, col);
	console_cursor_save(this->con);
	console_flush(this->con);
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
			ASSERT(id < state->client_count, "invalid id: %d, clientcount:%d", id, state->client_count);
			state->context[id]->effectflag = uimsg->flag;
			state->context[id]->color = uimsg->fgcolor;
			break;
		}
		case UIMSG_MOVE: {
			int id = uimsg->id;
			ASSERT(id < state->client_count, "invalid id: %d, clientcount:%d", id, state->client_count);
			state->context[id]->line = uimsg->line;
			state->context[id]->col = uimsg->col;
			break;
		}
		case UIMSG_MOVECURSOR: {
			int id = uimsg->id;
			ASSERT(id < state->client_count, "invalid id: %d, clientcount:%d", id, state->client_count);
			handle_movecursor(state, uimsg->line, uimsg->col);
			break;
		}
		case UIMSG_OUT: {
			ASSERT(uimsg->id < state->client_count, "invalid id: %d, clientcount:%d", uimsg->id, state->client_count);
			handle_uiout(state, uimsg);
			break;
		}
		case UIMSG_FORCE_REFRESH: {
			handle_force_refresh(state);
			break;
		}
		default:
			ASSERT(0, "unknown uimsg type: %d", uimsg->uimsg);
			return; // unreachable
	}
}

int uiserver_new() {
	int tid_server = Create(PRIORITY_UISERVER, uiserver);
	ASSERT(tid_server >= 0, "failed to create ui server");
	int tid_buf = buffertask_new(NAME_UISERVER, PRIORITY_UISERVER, MAX_UIMSGSIZE);
	ASSERT(tid_buf >= 0, "failed to create ui buffer");
	courier_new(PRIORITY_UISERVER, tid_buf, tid_server, MAX_UIMSGSIZE, NAME_UISERVER_C);
	return tid_buf;
}

ui_id uiserver_register_with_tid(int tid) {
	msg_ui uimsg;
	uimsg.type = MSG_UI;
	uimsg.uimsg = UIMSG_REGISTER;
	uimsg.id = MyTid();
	int rv = Send(tid, &uimsg, sizeof(uimsg), NULL, 0);
	ASSERT(rv >= 0, "send failed: rv:%d", rv);
	int tid_recv;
	int id;
	rv = Receive(&tid_recv, &id, sizeof(id));
	ASSERT(rv == sizeof(id), "receive failed: rv:%d", rv);
	rv = Reply(tid_recv, NULL, 0);
	ASSERT(rv >= 0, "reply failed");
	ui_id result;
	result.tid = tid;
	result.id = id;
	return result;
}

ui_id uiserver_register_blocking() {
	return uiserver_register_with_tid(WhoIs(NAME_UISERVER_S));
}

ui_id uiserver_register() {
	return uiserver_register_with_tid(WhoIs(NAME_UISERVER));
}


void uiserver_effect(ui_id id, int flag, int color) {
	msg_ui uimsg;
	uimsg.type = MSG_UI;
	uimsg.uimsg = UIMSG_EFFECT;
	uimsg.id = id.id;
	uimsg.flag = flag;
	uimsg.fgcolor = color;

	int rv = Send(id.tid, &uimsg, sizeof(uimsg), NULL, 0);
	ASSERT(rv >= 0, "send failed");
}

void uiserver_out(ui_id id, char *out) {
	int msg[MAX_UIMSGSIZE / sizeof(int)];
	msg_ui *uimsg = (msg_ui*) msg;
	uimsg->type = MSG_UI;
	uimsg->uimsg = UIMSG_OUT;
	uimsg->id = id.id;
	strncpy(uimsg->str, out, SCREEN_WIDTH);
	uimsg->str[SCREEN_WIDTH] = '\0';
	uimsg->strlen = min(SCREEN_WIDTH, strlen(out));

	int rv = Send(id.tid, uimsg, sizeof(msg), NULL, 0);
	ASSERT(rv >= 0, "send failed");
}

void uiserver_move(ui_id id, int line, int col) {
	msg_ui uimsg;
	uimsg.type = MSG_UI;
	uimsg.uimsg = UIMSG_MOVE;
	uimsg.id = id.id;
	uimsg.line = line;
	uimsg.col = col;

	int rv = Send(id.tid, &uimsg, sizeof(uimsg), NULL, 0);
	ASSERT(rv >= 0, "send failed");
}

void uiserver_movecursor(ui_id id, int line, int col) {
	msg_ui uimsg;
	uimsg.type = MSG_UI;
	uimsg.uimsg = UIMSG_MOVECURSOR;
	uimsg.id = id.id;
	uimsg.line = line;
	uimsg.col = col;

	int rv = Send(id.tid, &uimsg, sizeof(uimsg), NULL, 0);
	ASSERT(rv >= 0, "send failed");
}

void uiserver_force_refresh(ui_id id) {
	msg_ui uimsg;
	uimsg.type = MSG_UI;
	uimsg.uimsg = UIMSG_FORCE_REFRESH;

	int rv = Send(id.tid, &uimsg, sizeof(uimsg), NULL, 0);
	ASSERT(rv >= 0, "send failed");
}


