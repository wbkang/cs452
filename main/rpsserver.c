#include <rpsserver.h>
#include <syscall.h>
#include <rawio.h>
#include <hardware.h>

void rpsserver() {
	RegisterAs("rp");
	PRINT("herp derp");
}
