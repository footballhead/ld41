# Custom Makefile

CFLAGS=-Wall -Og -g -ggdb
AR=ar

.PHONY: all
all: monster repeater librepeater.a

monster: monster.o librepeater.a

librepeater.a: repeater_lib.o
	$(AR) $(ARFLAGS) $@ $^
