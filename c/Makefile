
LDFLAGS = $(shell pkg-config --libs glib-2.0) -L/usr/local/lib \
	  -O3
CFLAGS	= $(shell pkg-config --cflags glib-2.0) -I. -I/usr/local/include -Wall \
	  -O3 -DNDEBUG -fomit-frame-pointer
CC	= gcc
LD	= gcc

OBJS	= switch.o goto.o dasm-x64.o lightn.o
COMMON	= common.o

all: switch goto

more: all dasm-x64 lightn

clean:
	rm -f $(OBJS) $(COMMON) goto switch dasm-x64 dasm-x64.c lightn

goto:	goto.o $(COMMON) common.h
	$(LD) -o $@ $< $(COMMON) $(LDFLAGS)

switch:	switch.o $(COMMON) common.h
	$(LD) -o $@ $< $(COMMON) $(LDFLAGS)

dasm-x64:	dasm-x64.o $(COMMON) common.h
	$(LD) -o $@ $< $(COMMON) $(LDFLAGS)

dasm-x64.c: dasm-x64.dasm
	luajit dynasm/dynasm.lua -D X64 -o dasm-x64.c dasm-x64.dasm

lightn:	lightn.o $(COMMON) common.h
	$(LD) -o $@ $< $(COMMON) $(LDFLAGS) -llightning -ldl
