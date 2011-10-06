#include <nameserver.h>
#include <syscall.h>
#include <rawio.h>
#include <hardware.h>

#define NUM_ASCII_PRINTABLE (0x7e - 0x20 + 1)
#define ASCII_PRINTABLE_START 0x20

void nameserver() {
	bwprintf(COM2, "setting nameserver tid to %d\n", MyTid());
	// receive args
	int id;
	int req;
	// internal args
	int len;
	int rv;
	int hashsize = NUM_ASCII_PRINTABLE * NUM_ASCII_PRINTABLE; // two ASCII printable chars
	int mem[hashsize];
	int name;

	for (int i = 0; i < hashsize; i++) {
		mem[i] = -1;
	}

	for (;;) {
		bwprintf(COM2, "receiving...\n");
		len = Receive(&id, (void*) &req, sizeof(req));
		int reqno = GET_NAMESERVER_REQNUM(req);
		char reqname[3];
		reqname[0] = req >> 24;
		reqname[1] = (req >> 16);
		reqname[2] = '\0';

		if (len != sizeof(req)) {
			rv = -3; // error during copying
		} else {
			bwprintf(COM2, "[request] id: %d, str: '%s' %x\n", id, reqname, req);
			name = (int) (reqname[0] - ASCII_PRINTABLE_START) * NUM_ASCII_PRINTABLE + (reqname[1] - ASCII_PRINTABLE_START);
			switch (reqno) {
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
