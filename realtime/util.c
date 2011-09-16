#include <util.h>

#include <rawio.h>

void logmsg(char* msg)
{
	raw_logemergency(COM2,"LOG:");
	raw_logemergency(COM2, msg);
	raw_logemergency(COM2,"\n\r");
}


void die()
{
	//stub
	raw_logemergency(COM2,"PRETEND YOU ARE DEAD!");
	while(1);
}
