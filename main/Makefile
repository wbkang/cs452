#
# Makefile for busy-wait IO library
#
CCDIR = /u/wbcowan/gnuarm-4.0.2/arm-elf/bin

XCC	= $(CCDIR)/gcc
AS	= $(CCDIR)/as
AR	= $(CCDIR)/ar
LD  = $(CCDIR)/ld
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

LDFLAGS = -init main -Map main.map -N  -T orex.ld -L/u/wbcowan/gnuarm-4.0.2/lib/gcc/arm-elf/4.0.2 -L../lib

%.o : %.c

#################
OBJS= main.o test.o kerneltest.o task1.o k3clients.o
################


all : main.elf

main.elf : $(OBJS)
	@echo "[LD]\t$(OBJS) -> $@"
	@$(LD) $(LDFLAGS) -o $@ $(OBJS) -lkernel -lgcc

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
