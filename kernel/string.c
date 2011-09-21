#include<string.h>

unsigned int strlen(char*c)
{
	unsigned int count = 0;

	while(*(c++)) count++;

	return count;
}
