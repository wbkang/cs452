#include <a0.h>
#include <train.h>
#include <syscall.h>
#include <sensornotifier.h>

void a0() {
	int tid_time = WhoIs(NAME_TIMESERVER);
	int tid_com1 = WhoIs(NAME_IOSERVER_COM1);
	int tid_com2 = WhoIs(NAME_IOSERVER_COM2);

	train_go(tid_com1);

	// ioprintf(tid_com2, "switching to curved\n");
	// train_switchall('C', tid_com1, tid_time);
	// Delay(100, tid_time);

	int train = 24;
	int speed = 14;
	train_speed(train, speed, tid_com1);
	/*Delay(500, tid_time);
	train_speed(train, 0, tid_com1);
	train_reverse(train, tid_com1, tid_time);
	train_speed(train, speed, tid_com1);*/

	Create(2, sensornotifier);

	for (;;) {
		int tid;
		sensornotifier_msg sensor_msg;
		Receive(&tid, (void*) &sensor_msg, sizeof(sensor_msg));
		ReplyNull(tid);
		ioprintf(tid_com2, "%c: %d\n", sensor_msg.module, sensor_msg.id);
	}
}
