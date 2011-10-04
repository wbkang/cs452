#include <nameserver.h>
#include <syscall.h>
#include <bwio.h>
#include <hardware.h>

void nameserver() {
	// recieve args
	int id;
	nameserver_request req;
	int size = sizeof nameserver_request;
	// internal args
	int len;
	int req_no;

	for (;;) {
		len = Recieve(&id, req, size);
		if (len == size) {

		} else {

		}

		bwprintf(COM2, "%d", len);
		return;

		rv = 0xbeef; // look up the name
	}
}
