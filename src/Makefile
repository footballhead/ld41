# Custom Makefile

CFLAGS=-Wall -Og -g -ggdb

.PHONY: all
all: monsterd libplayermsg.a rpgstatsd librpgstats.a

monsterd: monsterd.o libplayermsg.a librpgstats.a

rpgstatsd: rpgstatsd.o libplayermsg.a

libplayermsg.a: playermsg.o
	$(AR) $(ARFLAGS) $@ $^

librpgstats.a: rpgstats.o
	$(AR) $(ARFLAGS) $@ $^

.PHONY: clean
clean:
	rm -f monsterd monsterd.o \
		rpgstatsd rpgstatsd.o \
		libplayermsg.a playermsg.o \
		librpgstats.a rpgstats.o
