
#include <train.h>
#include <ts7200.h>
#include <util.h>
#include <bwio.h>
#include <timer.h>

#define TRAIN_PORT COM1

#define STOP 0x61
#define GO 0x60
#define SWITCH_STRAIGHT 0x21
#define SWITCH_CURVED 0x22
#define TURN_OFF_SOLENOID 0x20


#define NUM_TRAINS 255

static int TRAIN_SPEED[NUM_TRAINS] = {0};

static void set_stopbit_2()
{
	int* uart1high = (int*) UART1_BASE + UART_LCRH_OFFSET;
	 *uart1high |= STP2_MASK;
}

static int train_write(int b)
{
	bwprintf(COM2, "WRITING %x" CRLF, b);
	sleep(500);
	return bwputc(TRAIN_PORT, b);
}

void train_init() 
{
	set_stopbit_2();
	bwsetspeed(TRAIN_PORT, 2400);
	bwsetfifo(TRAIN_PORT, OFF);
}

void train_stop()
{
	train_write(STOP);
}
void train_go()
{
	train_write(GO);
}

void train_setspeed(int train, int speed)
{
	bwprintf(COM2, "WRITING %x, %x" CRLF, speed, train);
	train_write(speed);
	train_write(train);
	TRAIN_SPEED[train] = speed;
	bwprintf(COM2, "Saving speed %x for train %x" CRLF, speed, train);
}

void train_reverse(int train)
{
	bwprintf(COM2, "WRITING 0, %x" CRLF, train);
	train_write(0);
	train_write(train);
	sleep(3000);
	bwprintf(COM2, "WRITING f, %x" CRLF, train);
	train_write(0xf);
	train_write(train);
	sleep(1000);
	bwprintf(COM2, "WRITING %x, %x" CRLF, TRAIN_SPEED[train], train);
	train_write(TRAIN_SPEED[train]);
	train_write(train);
}

void train_setswitch(int sw, enum switch_state state)
{
	if (state == straight) {
		train_write(SWITCH_STRAIGHT);
	}
	else if (state == curved) {
		train_write(SWITCH_CURVED);
	}
	else{
		ASSERT(0, "Wrong switch state");
	}

	train_write(sw);
	
	sleep(150);

	train_write(TURN_OFF_SOLENOID);
}

