#include <nameserver.h>
#include <syscall.h>
#include <rawio.h>
#include <hardware.h>

void nameserver() {
	#define IDX(s) (((s[0] - 'a') * 26) | (s[1] - 'a'))
	// recieve args
	int tid;
	nameserver_request req;
	// internal args
	int maxidx = IDX("zz"); // minidx = IDX("aa") = 0
	int mem[maxidx + 1];
	int name;
	for (name = 0; name <= maxidx; name++) mem[name] = -1;
	int rv;

	for (;;) {
		if (Recieve(&tid, (void*) &req, sizeof(req)) != sizeof(req)) {
			rv = -4; // error during copying
		} else {
			name = IDX(req.str);
			/*bwprintf(COM2, "[nsrequest] ");
			bwprintf(COM2, "type: %s, ", req.n ? "whois" : "reg");
			bwprintf(COM2, "tid: %d, ", tid);
			bwprintf(COM2, "name: %d, ", name);
			bwprintf(COM2, "mem: %d, ", mem[name]);
			bwprintf(COM2, "str: %c%c\n", req.str[0], req.str[1]);*/
			switch (req.n) {
				case NAMESERVER_REQUEST_REGISTERAS:
					if (mem[name] == -1) {
						mem[name] = tid;
						rv = 0;
					} else {
						rv = -5; // name already taken
					}
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
		//bwprintf(COM2, "rv: %d (%x)\n", rv, rv);
		Reply(tid, (void*) &rv, sizeof rv);
	}
}
