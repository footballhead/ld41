# Custom Makefile

CFLAGS=-Wall -Og -g -ggdb

.PHONY: all
all: monster repeater librepeater.a

monster: monster.o librepeater.a

librepeater.a: repeater_lib.o
	$(AR) $(ARFLAGS) $@ $^

.PHONY: clean
clean:
	rm -f monster repeater librepeater.a monster.o repeater_lib.o
