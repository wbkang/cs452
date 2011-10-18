#include <nameserver.h>
#include <syscall.h>
#include <rawio.h>
#include <hardware.h>
#include <constants.h>

typedef struct _tag_nameserver_req {
	char no;
	char ch[2];
} nameserver_req;

inline int nameserver_goodchar(char ch) {
	return ASCII_PRINTABLE_START <= ch && ch <= ASCII_PRINTABLE_END;
}

inline int nameserver_validname(char *name) {
	return name && nameserver_goodchar(name[0]) && nameserver_goodchar(name[1]) && name[2] == '\0';
}

void nameserver() {
	// init state
	int hashsize = NUM_ASCII_PRINTABLE * NUM_ASCII_PRINTABLE; // two chars
	ASSERT(hashsize < STACK_SIZE / sizeof(int), "not enough user memory");
	int mem[hashsize];
	for (int i = 0; i < hashsize; i++) {
		mem[i] = -1;
	}
	// init com args
	int tid;
	nameserver_req req;
	// serve
	int rv;
	for (;;) {
		int bytes = Receive(&tid, (void*) &req, sizeof(req));
		if (bytes == sizeof(req)) {
			int idx = req.ch[0] * NUM_ASCII_PRINTABLE + req.ch[1] - (NUM_ASCII_PRINTABLE + 1) * ASCII_PRINTABLE_START;
			switch (req.no) {
				case NAMESERVER_REGISTERAS:
					mem[idx] = tid;
					rv = 0;
					break;
				case NAMESERVER_WHOIS:
					if (mem[idx] == -1) {
						rv = NAMESERVER_ERROR_NOTREGISTERED;
						ASSERT(FALSE, "omfg looking for the name %c%c", req.ch[0], req.ch[1]);
					} else {
						rv = mem[idx];
					}
					break;
				default:
					rv = NAMESERVER_ERROR_BADREQNO;
					break;
			}
		} else {
			rv = NAMESERVER_ERROR_BADDATA;
		}
		Reply(tid, (void*) &rv, sizeof rv);
	}
}

inline int nameserver_send(char reqno, char *name) {
	if (!nameserver_validname(name)) return NAMESERVER_ERROR_BADNAME;
	nameserver_req req;
	req.no = reqno;
	req.ch[0] = name[0];
	req.ch[1] = name[1];
	int rv;
	int len = Send(NameServerTid(), (void*) &req, sizeof req, (void*) &rv, sizeof rv);
	if (len < 0) return len;
	if (len != sizeof rv) return NAMESERVER_ERROR_BADDATA;
	return rv;
}

/*
 * API
 */

inline int nameserver_registeras(char *name) {
	return nameserver_send(NAMESERVER_REGISTERAS, name);
}

inline int nameserver_whois(char *name) {
	return nameserver_send(NAMESERVER_WHOIS, name);
}
