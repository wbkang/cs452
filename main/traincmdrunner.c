#include <traincmdrunner.h>
#include <traincmdbuffer.h>
#include <syscall.h>

void traincmdrunner() {
	RegisterAs(NAME_TRAINCMDRUNNER);

	int tid_com1 = WhoIs(NAME_IOSERVER_COM1);
	// int tid_com2 = WhoIs(NAME_IOSERVER_COM2);
	int tid_traincmdbuf = WhoIs(NAME_TRAINCMDBUFFER);
	int tid_time = WhoIs(NAME_TIMESERVER);

	for (;;) {
		traincmd cmd;
		traincmdbuffer_get(tid_traincmdbuf, &cmd);
		switch (cmd.name) {
			case SPEED: {
				char train = cmd.byte1;
				char speed = cmd.byte2;
				Putc(COM1, speed, tid_com1);
				Putc(COM1, train, tid_com1);
				break;
			}
			case REVERSE: {
				char train = cmd.byte1;
				Putc(COM1, 0xF, tid_com1);
				Putc(COM1, train, tid_com1);
				break;
			}
			case SWITCH: {
				char switchaddr = cmd.byte1;
				char switchpos = cmd.byte2;
				Putc(COM1, switchpos == 'S' ? 0x21 : 0x22, tid_com1);
				Putc(COM1, switchaddr, tid_com1);
				break;
			}
			case SOLENOID:
				Putc(COM1, 0x20, tid_com1);
				break;
			case QUERY1: {
				char module = cmd.byte1;
				Putc(COM1, 0xC0 + module, tid_com1);
				break;
			}
			case QUERY: {
				char modules = cmd.byte1;
				Putc(COM1, 0x80 + modules, tid_com1);
				break;
			}
			case GO:
				Putc(COM1, 0x60, tid_com1);
				break;
			case STOP:
				Putc(COM1, 0x61, tid_com1);
				break;
			case PAUSE: {
				char ticks = cmd.byte1;
				Delay(ticks, tid_time);
				break;
			}
			default:
				ERROR("bad train cmd: %d", cmd.name);
				break;
		}
	}
}

int traincmdrunner_new() {
	return Create(PRIORITY_TRAINCMDRUNNER, traincmdrunner);
}
