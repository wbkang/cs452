#include <timeserver.h>
#include <syscall.h>

typedef struct _tag_timeserver_req {
	char no;
	int ticks;
} timeserver_req;

int timeserver_do_time() {
	return 0;
}

int timeserver_do_delayuntil(int ticks) {
	return 0;
}

void timeserver() {
	RegisterAs(TIMESERVER_NAME);
	int tid;
	timeserver_req req;
	int rv;
	for (;;) {
		rv = 0;
		if (sizeof(req) != Receive(&tid, (void*) &req, sizeof(req))) {
			rv = TIMESERVER_ERROR_BADDATA;
		} else {
			switch (req.no) {
				case TIMESERVER_TIME:
					rv = timeserver_do_time();
					break;
				case TIMESERVER_DELAYUNTIL:
					rv = timeserver_do_delayuntil(req.ticks);
					break;
				default:
					rv = TIMESERVER_ERROR_BADREQNO;
					break;
			}
		}
		if (rv <= 0) Reply(tid, (void*) &rv, sizeof rv); // return errors
	}
}

inline int timeserver_send(timeserver_req *req) {
	int server = WhoIs(TIMESERVER_NAME);
	if (server < 0) return server;
	int rv;
	int len = Send(server, (void*) req, sizeof(timeserver_req), (void*) &rv, sizeof rv);
	if (len != sizeof rv) return TIMESERVER_ERROR_BADDATA;
	return rv;
}

int timeserver_time() {
	timeserver_req req;
	req.no = TIMESERVER_TIME;
	return timeserver_send(&req);
}

int timeserver_delay(int ticks) {
	int time = timeserver_time();
	if (time < 0) return time;
	return timeserver_delayuntil(time + ticks);
}

int timeserver_delayuntil(int ticks) {
	timeserver_req req;
	req.no = TIMESERVER_DELAYUNTIL;
	req.ticks = ticks;
	return timeserver_send(&req);
}
