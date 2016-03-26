
LDFLAGS = $(shell pkg-config --libs glib-2.0) -O3
CFLAGS	= $(shell pkg-config --cflags glib-2.0) -Wall -O3 -fomit-frame-pointer
CC	= gcc
LD	= gcc

OBJS	= switch.o goto.o
COMMON	= common.o

all: switch goto

clean:
	rm -f $(OBJS) $(COMMON) goto switch

goto:	goto.o $(COMMON) common.h
	$(LD) -o $@ $< $(COMMON) $(LDFLAGS)

switch:	switch.o $(COMMON) common.h
	$(LD) -o $@ $< $(COMMON) $(LDFLAGS)
