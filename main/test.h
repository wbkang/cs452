#pragma once

#include <hardware.h>
#include <rawio.h>
#include <util.h>

#define TEST_START() { if (TEST_REALLY_ENABLED) { TRACE("[%s] test start", __func__)

#define TEST_END() TRACE("[%s] test end", __func__) } }

#define EXPECT(expected, got) { \
	int __e = (int)expected; int __g = (int)got; \
	if (__e != __g) { \
		PRINT(__FILE__ ":" TOSTRING(__LINE__) " ERROR!!!Expected %d but got %d (%x)", __e, __g, __g); \
		die(); \
	} \
}

#define EXPECTMSG(expected, got, MSG) { \
	int __e = (int)expected; int __g = (int)got; \
	if (__g != __e) { \
		PRINT(__FILE__ ":" TOSTRING(__LINE__) " ERROR!!!Expected %d but got %d (%x) " MSG, __e, __g, __g); \
		die(); \
	} \
}


void test_run();
