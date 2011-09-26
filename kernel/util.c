#include <util.h>
#include <hardware.h>
#include <rawio.h>

void die()
{
	//stub
	while(1);
}

void errormsg(char* msg)
{
	bwprintf(COM2, msg);
}
