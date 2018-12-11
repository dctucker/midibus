
CC=gcc
LIBS=-lasound -lpthread
SRCS=thru.c

all: thru

depend: .depend

.depend: $(SRCS)
	rm -f ./.depend
	$(CC) $(CFLAGS) -MM $^ > ./.depend;

include .depend

thru: thru.c thru.h
	$(CC) -o $@ $(LIBS) thru.c

rawmidi:
	${CC} -o rawmidi -lasound rawmidi.c

