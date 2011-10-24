#include <traincmdbuffer.h>
#include <queue.h>
#include <syscall.h>
#include <traincmdrunner.h>

#define LEN_CMDBUFF 32
#define LEN_TIDBUFF 1

typedef struct {
	queue *cmdq;
	queue *tidq;
} traincmdbuffer_state;

typedef struct {
	enum { PUT, GET } no;
	unsigned int cmd;
} traincmdbuffer_req;

typedef struct {
	unsigned int cmd;
} traincmdbuffer_reply;

static void tx(int tid, unsigned int cmd) {
	traincmdbuffer_reply rpl;
	rpl.cmd = cmd;
	Reply(tid, &rpl, sizeof(rpl));
}

static void handle_put(traincmdbuffer_state *state, int tid, unsigned int cmd) {
	ReplyNull(tid);
	if (queue_empty(state->tidq)) {
		queue_push(state->cmdq, (void*) cmd);
	} else {
		tx((int) queue_pop(state->tidq), cmd);
	}
}

static void handle_get(traincmdbuffer_state *state, int tid) {
	if (queue_empty(state->cmdq)) {
		queue_push(state->tidq, (void*) tid);
	} else {
		tx(tid, (unsigned int) queue_pop(state->cmdq));
	}
}

void traincmdbuffer() {
	RegisterAs(NAME_TRAINCMDBUFFER);

	traincmdbuffer_state state;
	state.cmdq = queue_new(LEN_CMDBUFF);
	state.tidq = queue_new(LEN_TIDBUFF);

	traincmdrunner_new();

	for (;;) {
		int tid;
		traincmdbuffer_req req;
		Receive(&tid, &req, sizeof(req));
		switch (req.no) {
			case PUT:
				handle_put(&state, tid, req.cmd);
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

static unsigned int encode(traincmd *cmd) {
	unsigned int rv = 0;
	rv |= (cmd->name) << 16;
	rv |= (cmd->byte1) << 8;
	rv |= cmd->byte2;
	return rv;
}

static void decode(unsigned int icmd, traincmd *cmd) {
	cmd->name = icmd >> 16;
	cmd->byte1 = (icmd >> 8) & 0xFF;
	cmd->byte2 = icmd & 0xFF;
}

/*
 * API
 */

int traincmdbuffer_new() {
	return Create(PRIORITY_TRAINCMDBUFFER, traincmdbuffer);
}

int traincmdbuffer_put(int tid, traincmdname name, char byte1, char byte2) {
	traincmd cmd;
	cmd.name = name;
	cmd.byte1 = byte1;
	cmd.byte2 = byte2;
	// send
	traincmdbuffer_req req;
	req.no = PUT;
	req.cmd = encode(&cmd);
	return Send(tid, (void*) &req, sizeof(req), NULL, 0);
}

int traincmdbuffer_get(int tid, traincmd *cmd) {
	traincmdbuffer_req req;
	req.no = GET;
	traincmdbuffer_reply rpl;
	int len = Send(tid, (void*) &req, sizeof(req), (void*) &rpl, sizeof(rpl));
	if (len < 0) return len;
	decode(rpl.cmd, cmd);
	return 0;
}
