#
# Makefile for busy-wait IO library
#
CCDIR = /u/wbcowan/gnuarm-4.0.2/arm-elf/bin

XCC     = $(CCDIR)/gcc
AS	= $(CCDIR)/as
AR	= $(CCDIR)/ar
CFLAGS  = -c -fPIC -Wall -I. -I../include -mcpu=arm920t -msoft-float -nostdlib -pedantic -Wall -std=c99

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
TARGETLIB= realtime.a
OBJS= bwio.o console.o train.o
################


all : $(TARGETLIB)

%.a : $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)


%.o : %.s
	$(AS) $(ASFLAGS) -o $@ $<

%.s : %.c
	$(XCC) -S $(CFLAGS)  $< 


clean:
	-rm -f *.s *.a *.o

.PRECIOUS: %.o %.a %.s %.c
