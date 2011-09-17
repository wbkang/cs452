#include <time.h>

struct timestruct time_convert(unsigned int ms)
{
	struct timestruct s;
	s.hour = ms / 3600000;
	s.min = (ms / 60000) % 60;
	s.sec = (ms / 1000) % 60;
	s.ds = (ms / 100) % 10;
	s.ms = ms % 1000;
	return s;
}

