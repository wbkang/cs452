#include <nameserver.h>
#include <syscall.h>
#include <rawio.h>
#include <hardware.h>
#include <constants.h>
#include <lookup.h>

typedef struct _tag_nameserver_req {
	char no;
	char ch[3];
} nameserver_req;

static inline int goodchar(char ch) {
	return ASCII_PRINTABLE_START <= ch && ch <= ASCII_PRINTABLE_END;
}

static inline int nameserver_validname(char *name) {
	return name && goodchar(name[0]) && goodchar(name[1]) && name[2] == '\0';
}

static int name_hash(void* nameptr) {
	char *name = (char*) nameptr;
	if (!nameserver_validname(nameptr)) return -1;
	return name[0] * NUM_ASCII_PRINTABLE + name[1] - (NUM_ASCII_PRINTABLE + 1) * ASCII_PRINTABLE_START;
}

void nameserver() {
	// init state
	int hashsize = NUM_ASCII_PRINTABLE * NUM_ASCII_PRINTABLE; // two chars
	lookup *nametidmap = lookup_new(hashsize, name_hash, NULL);

	// init com args
	int tid;
	nameserver_req req;
	// serve
	int rv;
	for (;;) {
		int bytes = Receive(&tid, &req, sizeof(req));
		if (bytes == sizeof(req)) {
			switch (req.no) {
				case NAMESERVER_REGISTERAS:
					lookup_put(nametidmap, req.ch, (void*) tid);
					rv = 0;
					break;
				case NAMESERVER_WHOIS: {
					if (lookup_goodkey(nametidmap, req.ch)) {
						void* data = lookup_get(nametidmap, req.ch);
						if (data) {
							int registered_tid = (int) data;
							if (registered_tid >= 0) {
								rv = registered_tid;
								break;
							}
						}
					}
					rv = NAMESERVER_ERROR_NOTREGISTERED;
					ASSERT(FALSE, "name \"%c%c\" not found", req.ch[0], req.ch[1]);
					break;
				}
				default:
					rv = NAMESERVER_ERROR_BADREQNO;
					break;
			}
		} else {
			rv = NAMESERVER_ERROR_BADDATA;
		}
		Reply(tid, &rv, sizeof rv);
	}
}

static inline int nameserver_send(char reqno, char *name) {
	if (!nameserver_validname(name)) return NAMESERVER_ERROR_BADNAME;
	nameserver_req req;
	req.no = reqno;
	req.ch[0] = name[0];
	req.ch[1] = name[1];
	req.ch[2] = name[2];
	int rv;
	int len = Send(NameServerTid(), &req, sizeof req, &rv, sizeof rv);
	if (len < 0) return len;
	if (len != sizeof rv) return NAMESERVER_ERROR_BADDATA;
	return rv;
}

/*
 * API
 */

int RegisterAs(char *name) {
	return nameserver_send(NAMESERVER_REGISTERAS, name);
}

int WhoIs(char *name) {
	return nameserver_send(NAMESERVER_WHOIS, name);
}
