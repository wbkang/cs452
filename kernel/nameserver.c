#include <nameserver.h>
#include <syscall.h>
#include <rawio.h>
#include <hardware.h>
#include <constants.h>
#include <lookup.h>

#define MAX_NUM_WHOIS_BLOCKED 32
#define NUM_NAMES (NUM_ASCII_PRINTABLE * NUM_ASCII_PRINTABLE)

typedef struct _tag_nameserver_req {
	char no;
	char ch[3];
} nameserver_req;

typedef struct _tag_whois_blocked {
	int tid;
	int hash;
} whois_blocked;

typedef struct _tag_nameserver_state {
	lookup *nametidmap;
	whois_blocked blocked[MAX_NUM_WHOIS_BLOCKED];
	int num_blocked;
} nameserver_state;

static inline int goodchar(char ch) {
	return ASCII_PRINTABLE_START <= ch && ch <= ASCII_PRINTABLE_END;
}

static inline int nameserver_validname(char *name) {
	return name && goodchar(name[0]) && goodchar(name[1]) && name[2] == '\0';
}

static int name_hash(void* nameptr) {
	char *name = nameptr;
	if (!nameserver_validname(nameptr)) return -1;
	return name[0] * NUM_ASCII_PRINTABLE + name[1] - (NUM_ASCII_PRINTABLE + 1) * ASCII_PRINTABLE_START;
}

static int handle_register(nameserver_state *state, int tid, char *name) {
	if (!nameserver_validname(name)) return ReplyInt(tid, NAMESERVER_ERROR_BADNAME);

	void* data = lookup_get(state->nametidmap, name);
	ASSERT(!data, "name %s already registered to task %d", name, (int) data);

	lookup_put(state->nametidmap, name, (void*) tid);
	ReplyInt(tid, 0);

	int hash = name_hash(name);

	// release blocked tasks
	for (int i = 0; i < state->num_blocked;) {
		whois_blocked *blockee = &state->blocked[i];
		if (blockee->hash == hash) {
			ReplyInt(blockee->tid, tid);
			memcpy(blockee, &state->blocked[state->num_blocked], sizeof(whois_blocked));
			state->num_blocked--;
		} else {
			i++;
		}
	}

	return 0;
}

static int handle_whois(nameserver_state *state, int tid, char *name) {
	if (!nameserver_validname(name)) return ReplyInt(tid, NAMESERVER_ERROR_BADNAME);

	void* data = lookup_get(state->nametidmap, name);
	if (data) return ReplyInt(tid, (int) data);

	// ASSERT(!strcmp(name, "04"), "name %s has not been registered", name);

	ASSERT(state->num_blocked < MAX_NUM_WHOIS_BLOCKED, "too many blocked");
	whois_blocked *blockee = &state->blocked[state->num_blocked];
	blockee->tid = tid;
	blockee->hash = name_hash(name);
	state->num_blocked++;

	return 0;
}

static nameserver_state *cur_state;
void nameserver() {
	nameserver_state state;
	cur_state = &state; // for debugging
	state.nametidmap = lookup_new(NUM_NAMES, name_hash, NULL);
	state.num_blocked = 0;

	for (;;) {
		int tid;
		nameserver_req req;
		int bytes = Receive(&tid, &req, sizeof(req));
		if (bytes == sizeof(req)) {
			switch (req.no) {
				case NAMESERVER_REGISTERAS:
					handle_register(&state, tid, req.ch);
					break;
				case NAMESERVER_WHOIS:
					handle_whois(&state, tid, req.ch);
					break;
				default:
					ReplyInt(tid, NAMESERVER_ERROR_BADREQNO);
					break;
			}
		} else {
			ReplyInt(tid, NAMESERVER_ERROR_BADDATA);
		}
	}
}

static inline int nameserver_send(char reqno, char *name) {
	if (!nameserver_validname(name)) return NAMESERVER_ERROR_BADNAME;
	nameserver_req req;
	req.no = reqno;
	strcpy(req.ch, name);
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

// should run absolutely positively only during crash
char* nameserver_get_name(int tid) {
	DEFINE_NAME_ALLNAMES(debug_names, debug_names_len);
	lookup *nametidmap = cur_state->nametidmap;
	for (int i = 0; i < debug_names_len; i++) {
		int tidfound = (int) lookup_get(nametidmap, debug_names[i][1]);
		if (tidfound == tid) return debug_names[i][0] + 5;
	}
	return NULL;
}
