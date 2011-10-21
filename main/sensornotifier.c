#include <sensornotifier.h>
#include <syscall.h>
#include <train.h>
#include <util.h>

#define NUM_MODULES 5

void sensornotifier() {
	int tid_com1 = WhoIs(NAME_IOSERVER_COM1);
	int tid_prnt = MyParentsTid();

	int modules[NUM_MODULES];

	for (int m = 0; m < NUM_MODULES; m++) {
		modules[0] = 0;
	}

	for (;;) {
		train_querysenmods(NUM_MODULES, tid_com1);
		for (int m = 0; m < NUM_MODULES; m++) {
			int upper = Getc(tid_com1);
			int lower = Getc(tid_com1);
			int module = (upper << 8) | lower;
			int old_module = modules[m];
			modules[m] = module;
			int sensors = (old_module ^ module) & (~old_module);
			while (sensors) {
				int s = log2(sensors);
				sensors &= ~(1 << s);
				sensornotifier_msg msg;
				msg.module = 'A' + m;
				msg.id = 16 - s;
				Send(tid_prnt, (void*) &msg, sizeof(msg), NULL, 0);
			}
		}
	}
}
