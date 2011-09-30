#pragma once

#include <hardware.h>
#include <rawio.h>
#include <util.h>

#define TEST_START(name) TRACE("\t%s ... ", name)

#define TEST_END(name) TRACE("looks good\n")

#define EXPECT(expected, got) { \
	if (got != expected) { \
		bwprintf(COM2, "ERROR!!!\nExpected %d but got %d (%x)", expected, got, got); \
		die(); \
	} \
}


void test_run();
