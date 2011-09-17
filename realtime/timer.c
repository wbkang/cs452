
#include <timer.h>
#include <ts7200.h>
#include <util.h>

#define TIMEOUT_CAPACITY 512 

static struct timeout_info timeout_ary[TIMEOUT_CAPACITY];
static int timeout_count;

int timer_timeoutcount()
{
	return timeout_count;
}

struct timeout_info timer_poptimeout()
{
	ASSERT(timeout_count>= 0 && timeout_count<= TIMEOUT_CAPACITY, "something crazy");

	if (timeout_count == 0)
	{
		struct timeout_info empty_timeout;
		empty_timeout.callback = NULL;
		empty_timeout.timeout = NULL;
		empty_timeout.start_time = NULL;
		empty_timeout.timeout = NULL;
		//logmsg("no timeout found");
		return empty_timeout;
	}
	else
	{
		struct timeout_info* to = NULL;
		unsigned int curtime = timer3_getvalue();
		
		for (int i = 0 ; i < TIMEOUT_CAPACITY; i++) {
			struct timeout_info * t = timeout_ary + i;
			
			if (t->callback && t->start_time + t->timeout <= curtime) {
				ASSERT(t->start_time, "invalid start_time");

				//logmsg("timeout found");
				//lognum(t->start_time);
				//lognum(t->timeout);
				//lognum(curtime);
				//lognum(t->callback);
				ASSERT(0 < (int)t->callback, "invalid callback!?");
				to = t;
				break;
			}
		}
		
		if (!to)
		{
			struct timeout_info empty_timeout;
			empty_timeout.callback = NULL;
			empty_timeout.timeout = NULL;
			empty_timeout.start_time = NULL;
			empty_timeout.timeout = NULL;
			//logmsg("no timeout found");
			return empty_timeout;
		}

		struct timeout_info retVal = *to;
		ASSERT(0 < (int)retVal.callback, "invalid callback!?");

		timeout_count--;
		to->callback = NULL;
		to->arg= NULL;
		to->start_time = NULL;
		to->timeout = NULL;

		return retVal;
	}
}

void timer3_init()
{
	// 2khz timer
	MEM(TIMER3_BASE + CRTL_OFFSET) &= ~CLKSEL_MASK;
	MEM(TIMER3_BASE + CRTL_OFFSET) |= ENABLE_MASK;
	
	timeout_count = 0;

	for (int i = 0; i < TIMEOUT_CAPACITY; i++) {
		timeout_ary[i].callback = NULL;
		timeout_ary[i].start_time= NULL;
		timeout_ary[i].arg= NULL;
		timeout_ary[i].timeout= NULL;
	}
}

void timer_cleartimeout(int id)
{
	if (id == 0)
	{
		//logmsg("cleartimeout: id 0");
		return;
	}

	ASSERT(timeout_count > 0, "there is no timeout to cancel!?!?");

	//logmsg("cleartimeout: id");
	//lognum(id);

	struct timeout_info *t = timeout_ary + id - 1;
	
	ASSERT(t->callback, "there is no callback timeout attached!");
	ASSERT(0 < (int) t->callback, "suspicious callback timeout attached!");
	t->callback = NULL;
	t->arg = NULL;
	t->start_time = NULL;
	t->timeout = NULL;

	timeout_count--;
}

int timer_settimeout(void (*fn)(void*), void* arg, unsigned int timeoutms)
{
	ASSERT(timeout_count >= 0, "something crazy");
	ASSERT(timeout_count <= TIMEOUT_CAPACITY, "too many timeouts!");
	ASSERT(fn, "there is no callback timeout attached!");
	if (0 <=(int) fn)
	{
		//logmsg("sus callback");
		//lognum(fn);
	}
	ASSERT(0 < (int) fn, "suspicious callback timeout attached!");


	struct timeout_info *spareto = NULL;
	int spareidx = -1;

	for (int i = 0; i< TIMEOUT_CAPACITY; i++)
	{
		if (timeout_ary[i].callback == NULL)
		{
			spareidx = i;
			spareto = timeout_ary + i;
			break;
		}
		else
		{
			ASSERT(0 < (int) timeout_ary[i].callback, "suspicious callback timeout attached!");
		}
	}

	ASSERT(spareto, "failed to find an empty timeout slot");

	spareto->callback = fn;
	spareto->arg = arg;
	spareto->start_time = timer3_getvalue();
	spareto->timeout = timeoutms;

	timeout_count++;

	return spareidx + 1;
}

unsigned int timer3_getvalue()
{
	return (0xffffffff - MEM(TIMER3_BASE + VAL_OFFSET)) >> 1;
}

// busy sleep
void sleep(int ms)
{
	unsigned int cur_time = timer3_getvalue();
	// 2khz timer	
	unsigned int actual_tick = ms;

	while (timer3_getvalue() < cur_time + actual_tick);

	return;
}
