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


%.o : %.c
#################
TARGETLIB= kernel.a
OBJS= util.o rawio.o hardware.o task.o memory.o kernel.o interrupt.o \
	heap.o stack.o syscall.o priorityq.o queue.o scheduler.o nameserver.o timeserver.o notifier.o \
	ioserver.o buffer.o
################


all : $(TARGETLIB)

%.a : $(OBJS)
	@echo "[AR]\t$< -> $@"
	@$(AR) $(ARFLAGS) $@ $(OBJS)

interrupt.o : interrupt.S
	@echo "[AS]\t$< -> $@"
	@$(AS) $(ASFLAGS) -o $@ $<

%.o : %.s
	@echo "[AS]\t$< -> $@"
	@$(AS) $(ASFLAGS) -o $@ $<

%.s : %.c
	@echo "[CC]\t$< -> $@"
	@$(XCC) -S $(CFLAGS)  $<


clean:
	@echo "[rm] Cleaning"
	@-rm -f *.s *.a *.o

.PRECIOUS: %.o %.a %.s %.c
