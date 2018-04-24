# Custom Makefile

CFLAGS=-Wall -Og -g -ggdb

.PHONY: all
all: skeletond libplayermsg.a rpgstatsd librpgstats.a riddlerd teddyd

skeletond: skeletond.o libplayermsg.a librpgstats.a

rpgstatsd: rpgstatsd.o libplayermsg.a

riddlerd: riddlerd.o libplayermsg.a

teddyd: teddyd.o libplayermsg.a

libplayermsg.a: playermsg.o
	$(AR) $(ARFLAGS) $@ $^

librpgstats.a: rpgstats.o
	$(AR) $(ARFLAGS) $@ $^

.PHONY: clean
clean:
	rm -f skeletond skeletond.o \
		rpgstatsd rpgstatsd.o \
		libplayermsg.a playermsg.o \
		librpgstats.a rpgstats.o
