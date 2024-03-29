#
# Makefile for busy-wait IO library
#
CCDIR = /u/wbcowan/gnuarm-4.0.2/arm-elf/bin

XCC	= $(CCDIR)/gcc
AS	= $(CCDIR)/as
AR	= $(CCDIR)/ar
LD  = $(CCDIR)/ld
CFLAGS  = -ggdb -fno-builtin -c -fPIC -Wall -I./include -I../include -mcpu=arm920t -msoft-float -nostdlib -pedantic -Wall -std=c99 -O3 -fno-omit-frame-pointer

# -g: include hooks for gdb
# -c: only compile
# -mcpu=arm920t: generate code for the 920t architecture
# -fpic: emit position-independent code
# -Wall: report all warnings
# -msoft-float: use software for floating point

ASFLAGS	= -mno-fpu -mcpu=arm920t -mapcs-32
# -mapcs-32: always create a complete stack frame

ARFLAGS = rcs

LDFLAGS = -init main -Map main.map -N -T orex.ld -L/u/wbcowan/gnuarm-4.0.2/lib/gcc/arm-elf/4.0.2 -L../lib

%.o : %.c

#################
OBJS= main.o test.o dumbbus.o train_calibrator.o train_stopper.o \
server/sensorserver.o server/comnotifier.o server/traincmdbuffer.o \
server/traincmdrunner.o server/timenotifier.o server/buffertask.o \
server/timeserver.o server/ioserver.o server/eventnotifier.o server/uiserver.o \
track_data.o track_node.o console.o \
task/calib_test.o task/task1.o task/k3clients.o task/a0.o task/perfmon.o task/accel.o task/decel.o \
ui/logstrip.o ui/cmdline.o ui/logdisplay.o ui/a0_track_template.o ui/timedisplay.o ui/a0ui.o \
engineer.o location.o server/courier.o server/publisher.o train.o track_reservation.o \
gps.o server/switchcmdrunner.o train_registry.o poly.o
################


all : main.elf

main.elf : $(OBJS) funcmap.o
	@echo "[LD]\t$(OBJS) -> $@"
	@$(LD) $(LDFLAGS) -o $@ $(OBJS) funcmap.o -lkernel -lgcc

funcmap.o : generate_funcmap
	@echo "[ASCC]\tfuncmap.c -> funcmap.o"
	@$(XCC) -S $(CFLAGS) -I../kernel/include  funcmap.c
	@$(AS) $(ASFLAGS) -o $@ funcmap.s

generate_funcmap : $(OBJS)
	@echo "[FUNCMAP] Generating funcmap..."
	@(cd ../;sh ./generate_funcmap.sh)

%.o : %.c
	@echo "[ASCC]\t$< -> $@"
	@$(XCC) $(CFLAGS) -o $@ $<
	@$(XCC) $(CFLAGS) -S $<

clean:
	@echo "[rm]\tCleaning"
	@-rm -f *.s *.a *.o *.map

.PRECIOUS: %.o %.a %.s %.c
