# Makefile
ALL := led_getios led_putios

CC=${CROSS_COMPILE}gcc
CFLAGS_DBG=-D_REENTRANT -g -ggdb -gdwarf -O0 -Wall -Wextra
CFLAGS=-D_REENTRANT -Wall -Wextra

all: ${ALL}
CB_FILES := *.[ch]

src_getios = led_getios.c
obj_getios = $(src:.c=.o)

LDFLAGS =

led_getios: led_getios.o convenient.o
	$(CC) ${CFLAGS} -o $@ $^ $(LDFLAGS)
led_putios: led_putios.o convenient.o
	$(CC) ${CFLAGS} -o $@ $^ $(LDFLAGS)
# Add capabilities:
# led_putios needs to write to procfs; traditionally, we'd do it with sudo;
# ...the Linux capabilities model would be much nicer security-wise
	$(info Adding capabilities to binary executable led_putios now...)
	sudo setcap CAP_DAC_OVERRIDE+eip ./led_putios


# cb - C Beautifier ! uses indent- "beautifies" C code into the "Linux kernel style"
# Note! original source file(s) is overwritten
cb: ${CB_FILES}
	mkdir bkp 2> /dev/null; cp -f ${CB_FILES} bkp/
	indent -linux ${CB_FILES}

clean:
	rm -vf ${ALL} *.o
