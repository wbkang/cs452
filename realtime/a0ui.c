#include <a0ui.h>
#include <timer.h>
#include <console.h>
#include <util.h>
#include <time.h>
#include <train.h>
#include <string.h>

#define REFRESH_TICK 100
#define NUM_SENSORSET 5
#define SENSOR_HISTORY 21

#define SCR_WIDTH 80
#define SCR_HEIGHT 25
#define CURSORGUARD(X) { console_savecursor(1); X; console_savecursor(0); };

static int last_draw_time = 0;
static int last_sensor_response_time = 0;

static void a0ui_clock_redraw();
static void a0ui_switch_draw();
static void a0ui_switch_redraw(int sw, enum switch_state);
static void a0ui_sensor_watcher(void *);
static void a0ui_sensor_draw();
static void a0ui_sensor_redraw();
static void a0ui_log_draw();

static char keyboard_buffer[SCR_WIDTH];
static int keyboard_buffer_idx;

static int sensor_watcher_timeout;
static int last_sensor_buffer[NUM_SENSORSET];
static int sensor_buffer[NUM_SENSORSET];
static int sensor_buffer_idx;
static int sensor_history[SENSOR_HISTORY][2];
static int sensor_history_idx;
static int sensor_redraw_timeout;
static unsigned int train_sensor_req_count;


static void reseteffects()
{
	console_showcursor(0);
	console_effect(EFFECT_RESET);
}

static void a0ui_redraw(void* redrawerptr)
{
	int curtime = timer3_getvalue();

	void (*fn)(void) = (void(*)(void))(FPTR_OFFSET+(unsigned int)redrawerptr);
	ASSERT(!(0xff000000 & (unsigned int)redrawerptr), "invalid callback addr");	
	fn();

	int targettime = (curtime / REFRESH_TICK) * REFRESH_TICK + REFRESH_TICK;

	timer_settimeout(a0ui_redraw, redrawerptr, MAX(targettime - curtime, 0));
}

static void print_pad2(unsigned int val)
{
	console_putc(val / 10 + '0');
	console_putc(val % 10 + '0');
}

static void a0ui_log_draw() 
{
	reseteffects();
	console_effect(EFFECT_UNDERSCORE);
	
	console_move(SCR_HEIGHT-2, 1);
	for ( int i = 0; i < SCR_WIDTH; i++) {
		console_putc(' ');
	}

	reseteffects();
	console_effect(EFFECT_BRIGHT);
	console_move(SCR_HEIGHT -1, 1);
	console_putstr("Command> ");
}

static void a0ui_sensor_watcher(void* unused) 
{
	logmsg("a0ui_sensor_watcher: ");
	lognum(sensor_watcher_timeout);
	train_sensor_req(sensor_buffer_idx);
	train_sensor_req_count++;
	sensor_watcher_timeout = timer_settimeout(a0ui_sensor_watcher, NULL, 5000);
}

static void a0ui_sensor_draw() 
{
	int startX = SCR_WIDTH  / 2 + 1;

	reseteffects();
	console_effect(EFFECT_UNDERSCORE);
	console_effect(EFFECT_FG_GREEN);

	console_move(1, startX);

	console_putstr(" Sensors");
	
	for (int i = startX + 8; i < SCR_WIDTH - 17; i++)
	{
		console_putc(' ');
	}
}


static void a0ui_switch_draw()
{
	console_move(1,1);
	reseteffects();
	console_effect(EFFECT_UNDERSCORE);
	console_effect(EFFECT_FG_YELLOW);
	
	console_putstr(" Switches");
	
	for (int i = 0; i < SCR_WIDTH * 3 / 4 - 9; i++) {
		console_putc(' ');	
	}

	reseteffects();

	for (int i = 1; i<= 18; i++) {
		console_move(i + 1,1);
		console_putstr("Sw   ");
		print_pad2(i);
		console_putstr(": ");
		console_putstr(train_getswitch(i) == straight ? "- straight" : "~ curved");
	}
	
	int upper_switch_offset = 0x9a;
	for (int i = 0; i < 4; i++) {
		console_move(2 + i, 21);
		console_putstr("SW 0x");
		console_printf("%x: ", i + upper_switch_offset);
		console_putstr(train_getswitch(i) == straight ? "- straight" : "~ curved");
	}

}

static void a0ui_switch_redraw(int sw, enum switch_state ss)
{
	char* msg = ss == straight ? "- straight" : "~ curved  ";

	reseteffects();

	if (sw >= 1 && sw <= 18) 
	{
		console_move(sw + 1,10);
		console_putstr(msg);
	}
	else if (sw >= 0x9a && sw <= 0x9d)
	{
		console_move(sw - 0x99 + 1, 30);
		console_putstr(msg);
	}
}


static void a0ui_clock_redraw()
{
	struct timestruct t = time_convert(timer3_getvalue());

	console_effect(EFFECT_UNDERSCORE);
	console_effect(EFFECT_FG_BLUE);
	console_effect(EFFECT_BRIGHT);

	console_move(1,SCR_WIDTH - 17);	
	console_putstr(" Time: ");
	print_pad2(t.hour);
	console_putc(':');
	print_pad2(t.min);
	console_putc(':');
	print_pad2(t.sec);
	console_putc('.');
	console_putc(t.ds + '0');
}


void a0ui_start()
{
	console_showcursor(0);
	console_clear();
	last_draw_time = 0; 
	last_sensor_response_time = 0;
	
	a0ui_switch_draw();
	a0ui_sensor_draw();
	a0ui_log_draw();

	for (int i =0; i < SCR_WIDTH; i++)
	{
		keyboard_buffer[i] = 0;
	}

	keyboard_buffer_idx = 0;
	
	for (int i=0; i<NUM_SENSORSET; i++)
	{
		last_sensor_buffer[i] = 0;
	}

	sensor_buffer_idx = 0;
	sensor_watcher_timeout = 0;
	train_sensor_req_count = 0;
	a0ui_sensor_watcher(NULL);

	timer_settimeout(a0ui_redraw, (void*)(unsigned int)a0ui_clock_redraw, REFRESH_TICK);
	//timer_settimeout(a0ui_sensor_watcher, NULL, 10);

	for (int i =0; i<SENSOR_HISTORY; i++)
	{
		sensor_history[i][0] = -1;
		sensor_history[i][1] = -1;
		sensor_buffer[i] = 0;
	}

	sensor_history_idx = 0;
	sensor_redraw_timeout = 0;
}

static int findFirstNonDigit(char* cmd)
{
	for (int i =0, n=strlen(cmd); i<n; i++) {
		if (cmd[i] < '0' || cmd[i] > '9') {
			return i;
		}
	}

	return -1;
}

static int parseInt(char* cmd)
{
	char buf[8];
	int size = 0;
	int foundend = 0;

	for (int i=0; i<4; i++)
	{
		if (cmd[i] >= '0' && cmd[i] <= '9') {
			buf[i] = cmd[i];
			size++;
		}
		else {
			foundend = 1;
			break;
		}
	}

	if (!foundend || !size) { return -1; }

	int result = 0;
	int multiplier = 1;

	for (int i = size -1; i >= 0; i--) {
		result += (buf[i] - '0') * multiplier;
		multiplier *= 10;
	}

	return result;
}

static void parseCommand(char* cmd)
{
	reseteffects();
	console_effect(EFFECT_UNDERSCORE);
	console_move(SCR_HEIGHT-2, 1);
	console_putstr("\033[K");
	console_move(SCR_HEIGHT-2, 1);

	unsigned int length = strlen(cmd);

	if (length == 1 && cmd[0] == 'q')
	{
		signal_quit();
		return;
	}
	else if (length >= 3)
	{
		if (cmd[0] == 't' && cmd[1] == 'r')
		{
			cmd += 3;

			int train_number = parseInt(cmd);

			if (train_number >= 1 && train_number <= 80) {
				cmd += 1 + findFirstNonDigit(cmd);	

				int train_speed = parseInt(cmd);

				if (train_speed >= 0 && train_speed <= 0xe) {
					console_printf("Successfully set the train %d's speed to %d.", train_number, train_speed);
					train_setspeed(train_number, train_speed);
					return;
				}
				else {
					console_printf("Invalid speed: '%s'", cmd);
					return;
				}
			}
			else {
				console_printf("Invalid train: '%s' %d", cmd, train_number);
				return;
			}
		}
		else if (cmd [0] == 'r' && cmd[1] == 'v')
		{
			cmd +=3;

			int train_number = parseInt(cmd);

			if (train_number >= 1 && train_number <= 80) {
				console_printf("Successfully reversed the train %d's direction.", train_number);
				train_reverse(train_number);
				return;
			}
			else {
				console_printf("Invalid speed: '%s'", cmd);
				return;
			}
		}
		else if (cmd[0] == 's' && cmd[1] == 'w')
		{
			cmd+=3;
			int switch_number = parseInt(cmd);

			if (switch_number >= 0 && switch_number <= 0xff)
			{
				cmd += 1 + findFirstNonDigit(cmd);	
			
				enum switch_state ss;

				switch(*cmd)
				{
					case 's':
					case 'S':
						ss = straight;
						break;
					case 'c':
					case 'C':
						ss = curved;
						break;
					default:
						console_printf("Invalid switch mode : '%c'", *cmd);
						return;
				}
				train_setswitch(switch_number, ss);
				console_printf("Successfully %s the switch #%d.", 
					ss == straight ? "straighted" : "bent", switch_number);
				a0ui_switch_redraw(switch_number, ss);
				return;
			}
			else
			{
				console_printf("Invalid switch: '%s'", cmd);
				return;
			}
		}

	}
	console_printf("Invalid comand: '%s'", cmd);
}

void a0ui_handleKeyInput(char c)
{
	reseteffects();
	console_effect(EFFECT_BRIGHT);
	const int max_buf_size = SCR_WIDTH - 10;
	
	if (c == '\b') 
	{
		if (keyboard_buffer_idx > 0)
		{
			console_move(SCR_HEIGHT-1, 10 + --keyboard_buffer_idx);
			console_putc(' ');
		}
	}
	else if (c == '\r')
	{
		console_move(SCR_HEIGHT-1, 10);
		for( int i = 0; i < keyboard_buffer_idx; i++) {
			console_putc(' ');
		}
		keyboard_buffer[keyboard_buffer_idx] = 0;
		parseCommand(keyboard_buffer);
		keyboard_buffer_idx = 0;
	}
	else if(keyboard_buffer_idx < max_buf_size)
	{
		console_move(SCR_HEIGHT-1, 10 + keyboard_buffer_idx);
		keyboard_buffer[keyboard_buffer_idx++]=c;
		console_putc(c);
	}
}

static void a0ui_sensor_redraw()
{
	int startX = SCR_WIDTH  / 2 + 1;
	int recentIdx = sensor_history_idx - 1;
	
	for (int i =0; i < 2; i ++) 
	{
		if (recentIdx < 0) { recentIdx = SENSOR_HISTORY-1; }

		int set = sensor_history[recentIdx][0];
		int sensor = sensor_history[recentIdx][1];

		char * fmtString = "Set %c Sensor %d        ";

		if (set != -1)
		{
			console_effect(EFFECT_RESET);

			if (i == 0) 
			{
				console_effect(EFFECT_BRIGHT);
			}

			console_move(1 + 1 + recentIdx, startX);
			console_printf(fmtString, set+ 'A', sensor);
		}

		recentIdx--;
	}

	

	sensor_redraw_timeout = 0;
}

static void a0ui_updateSensorHistory(int set, int sensor)
{
	sensor_history[sensor_history_idx][0] = set;
	sensor_history[sensor_history_idx][1] = sensor;

	if (++sensor_history_idx >= SENSOR_HISTORY) {
		sensor_history_idx = 0;
	}

	a0ui_sensor_redraw();
}

void a0ui_handleSensorInput(int b)
{
	if (train_sensor_req_count == 0)
	{
		//a0ui_updateSensorHistory(22, 0);
		return;
	}

	last_sensor_response_time = timer3_getvalue();
	sensor_buffer[sensor_buffer_idx] = b;
	
	int lastVal = last_sensor_buffer[sensor_buffer_idx];
	int turnedon = (b ^ lastVal) & ~lastVal;
	
	for (int j = 0; j < 16; j ++) 
	{
		if (turnedon % 2 == 1)
		{
			a0ui_updateSensorHistory(sensor_buffer_idx, 16-j);
		}

		turnedon = turnedon >> 1;
	}

	last_sensor_buffer[sensor_buffer_idx] = b;

	if (++sensor_buffer_idx >= NUM_SENSORSET) { sensor_buffer_idx = 0; }

	timer_cleartimeout(sensor_watcher_timeout);
	a0ui_sensor_watcher(NULL);
}



