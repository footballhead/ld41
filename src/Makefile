# Custom Makefile

CFLAGS=-Wall -Og -g -ggdb

.PHONY: all
all: monster librepeater.a rpgstatsd

monster: monster.o librepeater.a

rpgstatsd: rpgstatsd.o librepeater.a

librepeater.a: repeater_lib.o
	$(AR) $(ARFLAGS) $@ $^

.PHONY: clean
clean:
	rm -f monster repeater librepeater.a monster.o repeater_lib.o
