#pragma once

struct timestruct {
	unsigned int hour;
	unsigned int min;
	unsigned int sec;
	unsigned int ds;
	unsigned int ms;
};

struct timestruct time_convert(unsigned int ms);
