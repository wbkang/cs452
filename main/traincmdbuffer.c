#include <traincmdbuffer.h>
#include <queue.h>
#include <buffer.h>
#include <syscall.h>
#include <traincmdrunner.h>

#define LEN_CMDBUFF 64
#define LEN_TIDBUFF 1

typedef struct {
	buffer *cmdbuf;
	queue *tidq;
} traincmdbuffer_state;

typedef struct {
	enum { PUT, GET } no;
	traincmd cmd;
} traincmdbuffer_req;

static void handle_put(traincmdbuffer_state *state, int tid, traincmd *cmd) {
	ReplyNull(tid);
	if (queue_empty(state->tidq)) {
		buffer_put(state->cmdbuf, cmd);
	} else {
		Reply((int) queue_get(state->tidq), cmd, sizeof(*cmd));
	}
}

static void handle_get(traincmdbuffer_state *state, int tid) {
	if (buffer_empty(state->cmdbuf)) {
		queue_put(state->tidq, (void*) tid);
	} else {
		traincmd cmd;
		buffer_get(state->cmdbuf, &cmd);
		Reply(tid, &cmd, sizeof(cmd));
	}
}

void traincmdbuffer() {
	RegisterAs(NAME_TRAINCMDBUFFER);

	traincmdbuffer_state state;
	state.cmdbuf = buffer_new(LEN_CMDBUFF, sizeof(traincmd));
	state.tidq = queue_new(LEN_TIDBUFF);

	traincmdrunner_new();

	for (;;) {
		int tid;
		traincmdbuffer_req req;
		Receive(&tid, &req, sizeof(req));
		switch (req.no) {
			case PUT:
				handle_put(&state, tid, &req.cmd);
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

int traincmdbuffer_new() {
	return Create(PRIORITY_TRAINCMDBUFFER, traincmdbuffer);
}

int traincmdbuffer_put(int tid, traincmdname name, int arg1, int arg2) {
	traincmdbuffer_req req;
	req.no = PUT;
	req.cmd.name = name;
	req.cmd.arg1 = arg1;
	req.cmd.arg2 = arg2;
	return Send(tid, &req, sizeof(req), NULL, 0);
}

int traincmdbuffer_get(int tid, traincmd *cmd) {
	traincmdbuffer_req req;
	req.no = GET;
	int len = Send(tid, &req, sizeof(req), cmd, sizeof(*cmd));
	if (len < 0) return len;
	return 0;
}
