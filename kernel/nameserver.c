#include <nameserver.h>
#include <syscall.h>
#include <rawio.h>
#include <hardware.h>

static int nameserver_tid;

int get_nameserver() {
	return nameserver_tid;
}

void nameserver() {
	nameserver_tid = MyTid();
	bwprintf(COM2, "setting nameserver tid to %d\n", nameserver_tid);
	// recieve args
	int id;
	nameserver_request req;
	// internal args
	int len;
	int rv;
	int mem[255];
	int name;

	for (int i = 0; i < 255; i++) {
		mem[i] = -1;
	}

	for (;;) {
		bwprintf(COM2, "recieving...\n");
		len = Recieve(&id, (void*) &req, sizeof(req));
		if (len != sizeof(req)) {
			rv = -3; // error during copying
		} else {
			bwprintf(COM2, "[request] id: %d, str: '%s'\n", id, req.str);
			name = (int) req.str[0];
			switch (req.n) {
				case NAMESERVER_REQUEST_REGISTERAS:
					if (mem[name] == -1) {
						mem[name] = id;
						rv = 0;
					} else {
						rv = -4; // name already taken
					}
					break;
				case NAMESERVER_REQUEST_WHOIS:
					if (mem[name] == -1) {
						rv = -6; // name not registered
					} else {
						rv = mem[name];
					}
					break;
				default:
					rv = -5; // incorrect nameserver command
					break;
			}
		}
		bwprintf(COM2, "rv: %d (%x)\n", rv, rv);
		Reply(id, (void*) &rv, sizeof rv);
	}
}
