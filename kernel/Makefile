#
# Makefile for busy-wait IO library
#
CCDIR = /u/wbcowan/gnuarm-4.0.2/arm-elf/bin

XCC	= $(CCDIR)/gcc
AS	= $(CCDIR)/as
AR	= $(CCDIR)/ar
CFLAGS  = -fno-builtin -fomit-frame-pointer -c -fPIC -Wall -I. -I../include -mcpu=arm920t -msoft-float -nostdlib -pedantic -Wall -std=c99 -O2

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
OBJS= util.o rawio.o string.o hardware.o task.o memory.o kernel.o interrupt.o \
	heap.o stack.o syscall.o priorityq.o queue.o scheduler.o nameserver.o timeserver.o
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
