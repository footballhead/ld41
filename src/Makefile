# Custom Makefile

CFLAGS=-Wall -Og -g -ggdb

.PHONY: all
all: monsterd libplayermsg.a rpgstatsd

monsterd: monsterd.o libplayermsg.a

rpgstatsd: rpgstatsd.o libplayermsg.a

libplayermsg.a: playermsg.o
	$(AR) $(ARFLAGS) $@ $^

.PHONY: clean
clean:
	rm -f monsterd monsterd.o \
		rpgstatsd rpgstatsd.o \
		libplayermsg.a playermsg.o
