#include <util.h>

#include <rawio.h>

void logmsg(char* msg)
{
	if (!DEBUGMSG) 
	{
		return;
	}
	raw_logemergency(COM2,"LOG:");
	raw_logemergency(COM2, msg);
	raw_logemergency(COM2,"\n\r");
}

void lognum(int num)
{
	if (!DEBUGMSG)
	{
		return;
	}
	int idx = 0;
	char buf[1000] ;
	int neg = num < 0;

	if (neg)
	{
		num *= -1;
	}
	
	if (num == 0)
	{
		buf[idx++] = '0';
	}
	else
	{
		while (num > 0) {
			buf[idx++] = '0' + num % 10;
			num /= 10;
		}
	}

	char msgbuf[1000];
	int msgidx = 0;
	
	if (neg)
	{
		msgbuf[msgidx++] = '-';
	}

	while (--idx>=0) {
		msgbuf[msgidx++] = buf[idx];
	}
	
	msgbuf[msgidx] = '\0';

	raw_logemergency(COM2, "LOGNUM:");
	raw_logemergency(COM2, msgbuf);
	raw_logemergency(COM2, CRLF);
}


void die()
{
	//stub
	raw_logemergency(COM2,"PRETEND YOU ARE DEAD!");
	while(1);
}
