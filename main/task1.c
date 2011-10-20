#include <task1.h>
#include <syscall.h>
#include <timeserver.h>
#include <perfmon.h>
#include <k3clients.h>
#include <ioserver.h>
#include <train.h>

#define RPS_SERVER 0
#define PERFMON 0
#define K3 0

void task1() {
	int tid_time = Create(PRIORITY_TIMESERVER, timeserver);
	int tid_com1 = ioserver_create(COM1, OFF, 2400, 2, 8, OFF);
	int tid_com2 = ioserver_create(COM2, OFF, 115200, 1, 8, OFF);

	train_go(tid_com1);


	ioprintf(tid_com2, "switching to curved\n");
	train_switchall('C', tid_com1, tid_time);
	Delay(100, tid_time);

	int train = 24;
	int speed = 14;
	train_speed(train, speed, tid_com1);
	/*Delay(500, tid_time);
	train_speed(train, 0, tid_com1);
	train_reverse(train, tid_com1, tid_time);
	train_speed(train, speed, tid_com1);*/

	for (;;) {
		train_querysenmods(5, tid_com1);
		for (int i = 0; i < 5; i++) {
			int upper = Getc(tid_com1);
			int lower = Getc(tid_com1);
			int sensors = (upper << 16) | lower;
			ioprintf(tid_com2, "%c: %b    ", 'A' + i, sensors);
		}
		ioprintf(tid_com2, "\n");
	}

	// train_stop(tid_com1);

#if RPS_SERVER
	Create(1, rps_server);
	for (int i = 0; i < 4; i++) Create(1, rps_client);
#endif

#if PERFMON
	Create(1, perfmon);
#endif

#if K3
	Create(10, k3main);
#endif
}
