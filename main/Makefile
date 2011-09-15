#
# Makefile for busy-wait IO library
#
CCDIR = /u/wbcowan/gnuarm-4.0.2/arm-elf/bin

XCC     = $(CCDIR)/gcc
AS	= $(CCDIR)/as
AR	= $(CCDIR)/ar
LD  = $(CCDIR)/ld
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

LDFLAGS = -init main -Map main.map -N  -T orex.ld -L/u/wbcowan/gnuarm-4.0.2/lib/gcc/arm-elf/4.0.2 -L../lib

%.o : %.c

#################
OBJS= main.o
################


all : main.elf

main.elf : $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) -lrealtime -lgcc

%.o : %.s
	$(AS) $(ASFLAGS) -o $@ $<

%.s : %.c
	$(XCC) -S $(CFLAGS)  $< 

clean:
	-rm -f *.s *.a *.o *.map

.PRECIOUS: %.o %.a %.s %.c
