#pragma once

#include <hardware.h>
#include <rawio.h>
#include <util.h>

#define TEST_START() { if (TEST_REALLY_ENABLED) { TRACE("test start")
#define TEST_END() TRACE("test end") } }
#define KTEST_START() { if (KTEST_REALLY_ENABLED) { TRACE("ktest start")
#define KTEST_END() TRACE("ktest end") } }

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
