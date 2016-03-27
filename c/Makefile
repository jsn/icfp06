
LDFLAGS = $(shell pkg-config --libs glib-2.0) -L/usr/local/lib \
	  -g # -O3
CFLAGS	= $(shell pkg-config --cflags glib-2.0) -I/usr/local/include -Wall \
	  -g # -O3 -fomit-frame-pointer
CC	= gcc
LD	= gcc

OBJS	= switch.o goto.o
COMMON	= common.o

all: switch goto

clean:
	rm -f $(OBJS) $(COMMON) goto switch jit-dynasm jit-dynasm.c

goto:	goto.o $(COMMON) common.h
	$(LD) -o $@ $< $(COMMON) $(LDFLAGS)

switch:	switch.o $(COMMON) common.h
	$(LD) -o $@ $< $(COMMON) $(LDFLAGS)

jit-dynasm:	jit-dynasm.c $(COMMON)
	$(CC) -I. -o jit-dynasm jit-dynasm.c $(CFLAGS) $(COMMON) $(LDFLAGS)

jit-dynasm.c: jit-dynasm.dasm
	luajit dynasm/dynasm.lua -D X64 -o jit-dynasm.c jit-dynasm.dasm

jit-lightning:	jit-lightning.c $(COMMON)
	$(CC) -o $@ $< \
	    $(CFLAGS) $(COMMON) $(LDFLAGS) \
	    -llightning -ldl
