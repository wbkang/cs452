
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

#define SWITCH_GAP 200

#define NUM_TRAINS 80// 1 index 

static int TRAIN_SPEED[NUM_TRAINS+1] = {0};
static int TRAIN_REVERSED[NUM_TRAINS+1] = {0};
static int TRAIN_DELAYED_TIMEOUT[NUM_TRAINS+1] = {0};

static int SWITCH_STATUS[NUM_SWITCHES] = { 0} ;
static int solenoid_timeout;
static unsigned int last_switch_write_time = 0;

static void set_stopbit_2()
{
	int* uart1high = (int*) UART1_BASE + UART_LCRH_OFFSET;
	 *uart1high |= STP2_MASK;
}

static int train_write(int b)
{
	//bwprintf(COM2, "WRITING %x" CRLF, b);
	logmsg("train_write");
	lognum(b);
	return bwputc(TRAIN_PORT, b);
}

static void cancel_trainspeed_lambda(int train)
{
	timer_cleartimeout(TRAIN_DELAYED_TIMEOUT[train]);
	TRAIN_DELAYED_TIMEOUT[train] = 0;
}

void train_init() 
{
	set_stopbit_2();
	bwsetspeed(TRAIN_PORT, 2400);
	bwsetfifo(TRAIN_PORT, OFF);
	train_go();

	BOOTLOG("SETTING TRAIN SPEED TO 0"); 

	for (int i = 0; i <= NUM_TRAINS; i++) {
		TRAIN_SPEED[i] = 0;
		TRAIN_REVERSED[i] = 0;
		TRAIN_DELAYED_TIMEOUT[i] = 0;
		train_setspeed(i, 0);
	}

	BOOTLOG("SETTING ALL SWITCH TO CURVED"); 

	solenoid_timeout = NULL;
	last_switch_write_time = 0;

	for (int i = 0; i < NUM_SWITCHES; i++) {
		if (i <= 18 || i >= 0x99)
		{
			train_setswitch(i, curved);		
			//train_setswitch(i, straight);		
		}
	}

}

enum switch_state train_getswitch(int sw)
{
	return SWITCH_STATUS[sw];
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
	if (train < 1 || train >= NUM_TRAINS) {
		return;
	}

	//logmsg("train_setspeed");
	//lognum(train);
	//lognum(speed);

	train_write(speed);
	train_write(train);
	TRAIN_SPEED[train] = speed;
	cancel_trainspeed_lambda(train);
}

static void train_speed_lambda(void* p)
{
	// MSB is reverse bit
	// 15 bit followed is the train #
	// bottom 16 bit is the speed
	unsigned int data = (unsigned int)p;
	int reverse = 0x80000000 & data;
	int train = 0x7fff & (data >> 16);
	int speed = data & 0xffff; 
	
	if (reverse)
	{
		train_write(0xf);
		train_write(train);
	}

	train_write(speed);
	train_write(train);
	TRAIN_DELAYED_TIMEOUT[train] = NULL;
}

void train_reverse(int train)
{
	cancel_trainspeed_lambda(train);
	train_write(0);
	train_write(train);

	TRAIN_DELAYED_TIMEOUT[train] = timer_settimeout(
			train_speed_lambda, 
			(void*)(0x80000000 | (train<<16) | TRAIN_SPEED[train]), 3000);
}

void solenoid_off_lambda(void* unused)
{
	logmsg("solenoid_off_lambda");
	lognum((int)unused);
	train_write(TURN_OFF_SOLENOID);
	solenoid_timeout = NULL;
}

void train_turnoff_solenoid()
{
	timer_cleartimeout(solenoid_timeout);
	solenoid_timeout = timer_settimeout(solenoid_off_lambda, NULL, SWITCH_GAP + 100);
}

static void setswitch_lambda(void* raw_swstate)
{
	unsigned int swstate = (unsigned int) raw_swstate;
	int sw = swstate >> 16;
	enum switch_state state = swstate & 0xff;

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
	train_turnoff_solenoid();
	SWITCH_STATUS[sw] = state;
}

void train_setswitch(int sw, enum switch_state state)
{
	int curtime = timer3_getvalue();
	int trigger_time = MAX(last_switch_write_time + SWITCH_GAP, timer3_getvalue());
	last_switch_write_time = trigger_time;		
	int wait_time = MAX(trigger_time - curtime, 0);

	timer_settimeout(setswitch_lambda, (void*)((sw << 16) | (unsigned int) state), wait_time);
}


void train_batch_sensor_req(int n)
{
	ASSERT(n > 0, "need to prove at least 1 sensor");
	train_write(0x80 + n);
}

void train_sensor_req(int n)
{
	ASSERT(n >= 0, "negative sensor number!?!?");

	if (n > 12)
	{
		return; // invalid
	}

	train_write(0xc1 + n);
}
