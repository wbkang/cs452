#include <nameserver.h>
#include <syscall.h>
#include <rawio.h>
#include <hardware.h>

void nameserver() {
	// receive args
	int tid;
	int req;
	// internal args
	int hashsize = NUM_ASCII_PRINTABLE * NUM_ASCII_PRINTABLE; // two printable chars
	int mem[hashsize];
	for (int i = 0; i < hashsize; i++) mem[i] = -1;
	int name;
	int reqno;
	int rv;

	for (;;) {
		if (Receive(&tid, (void*) &req, sizeof(req)) != sizeof(req)) {
			rv = -3; // error during copying
		} else {
			reqno = NAMESERVER_GET_REQNO(req);
			name = NAMESERVER_GET_NAME(req);

//			bwprintf(COM2, "[nsrequest] ");
//			bwprintf(COM2, "req: %x, ", req, req);
//			bwprintf(COM2, "type: %s, ", reqno ? "whois" : "reg");
//			bwprintf(COM2, "tid: %d, ", tid);
//			bwprintf(COM2, "name: %d, ", name);
//			bwprintf(COM2, "mem: %d, ", mem[name]);
//			bwprintf(COM2, "str: %c%c\n", req >> 24, req >> 16);
			switch (reqno) {
				case NAMESERVER_REQUEST_REGISTERAS:
					mem[name] = tid;
					rv = 0;
					break;
				case NAMESERVER_REQUEST_WHOIS:
					if (mem[name] == -1) {
						rv = -5; // name not registered
					} else {
						rv = mem[name];
					}
					break;
				default:
					rv = -6; // incorrect nameserver command
					break;
			}
		}
		// bwprintf(COM2, "rv: %d (%x)\n", rv, rv);
		Reply(tid, (void*) &rv, sizeof rv);
	}
}
