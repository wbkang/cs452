#include <timenotifier.h>
#include <syscall.h>
#include <timeserver.h>

#define IRQ_TIMER1 0

void timenotifier() {
	int timeserver = WhoIs("TIMESERVER_NAME");
	for (;;) {
		int rv = AwaitEvent(IRQ_TIMER1);
		ASSERT(rv >= 0, "incorrect AwaitEvent return value");
		timeserver_tick();
	}
}
