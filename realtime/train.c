
#include <train.h>
#include <ts7200.h>
#include <util.h>
#include <bwio.h>

#define TRAIN_PORT COM1

#define STOP 0x61
#define GO 0x60
#define SWITCH_STRAIGHT 0x21
#define SWITCH_CURVED 0x22
#define TURN_OFF_SOLENOID 0x20

#define pause() { int i = 1000000; while(i-->0); }

#define NUM_TRAINS 255

static int TRAIN_SPEED[NUM_TRAINS] = {0};

static void set_stopbit_2()
{
	int* uart1high = (int*) UART1_BASE + UART_LCRH_OFFSET;
	 *uart1high |= STP2_MASK;
}

static int train_write(int b)
{
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
	train_write(speed);
	train_write(train);
	TRAIN_SPEED[train] = speed;
}

void train_reverse(int train)
{
	train_setspeed(train, 0);
	pause();
	train_write(0xf);
	train_write(train);
	pause();
	train_setspeed(train, TRAIN_SPEED[train]);
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
	
	pause();

	train_write(TURN_OFF_SOLENOID);

}

