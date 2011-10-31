#
# Makefile for busy-wait IO library
#
CROSSTOOLS_DIR = /u6/wbkang/opt/crosstool/gcc-4.0.1-glibc-2.3.5/arm-unknown-linux-gnu
CCDIR = $(CROSSTOOLS_DIR)/bin
LIBGCC_INCLUDE = $(CROSSTOOLS_DIR)/lib/gcc/arm-unknown-linux-gnu/4.0.1/include
LIBC_INCLUDE = $(CROSSTOOLS_DIR)/arm-unknown-linux-gnu/include

TARGET_ARCH = arm-unknown-linux-gnu
XCC = $(CCDIR)/$(TARGET_ARCH)-gcc
CC = $(XCC)
AS  = $(CCDIR)/$(TARGET_ARCH)-as
AR  = $(CCDIR)/$(TARGET_ARCH)-ar
CFLAGS  = -fomit-frame-pointer -c -fPIC -Wall \
	-I. -I../include -I$(GLIBC_INCLUDE) -I$(LIBGCC_INCLUDE) -I$(LIBC_INCLUDE) \
	-mcpu=arm920t -pedantic -Wall -std=c99 -O2

# -g: include hooks for gdb
# -c: only compile
# -mcpu=arm920t: generate code for the 920t architecture
# -fpic: emit position-independent code
# -Wall: report all warnings
# -msoft-float: use software for floating point

ASFLAGS	= -mcpu=arm920t -mapcs-32
# -mapcs-32: always create a complete stack frame

ARFLAGS = rcs

LDFLAGS = -init main -Map main.map -N  -T orex.ld \
	-L$(CROSSTOOLS_DIR)/lib/gcc/arm-unknown-linux-gnu/4.0.1 \
	-L$(CROSSTOOLS_DIR)/arm-unknown-linux-gnu/lib -L../lib


%.o : %.c

#################
OBJS= main.o test.o kerneltest.o task1.o k3clients.o a0.o sensornotifier.o comnotifier.o traincmdbuffer.o traincmdrunner.o timenotifier.o perfmon.o
################


all : main.elf

main.elf : $(OBJS)
	@echo "[LD]\t$(OBJS) -> $@"
	@$(LD) $(LDFLAGS) -o $@ $(OBJS) -lkernel -lc -lgcc

%.o : %.s
	@echo "[AS]\t$< -> $@"
	@$(AS) $(ASFLAGS) -o $@ $<

%.s : %.c
	@echo "[CC]\t$< -> $@"
	@$(XCC) -S $(CFLAGS)  $<

clean:
	@echo "[rm]\tCleaning"
	@-rm -f *.s *.a *.o *.map

.PRECIOUS: %.o %.a %.s %.c
