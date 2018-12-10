
CC=gcc
LIBS=-lasound -lpthread
DEPS=thru.h

thru: thru.c ${DEPS}
	${CC} -o $@ ${LIBS} thru.c

rawmidi:
	${CC} -o rawmidi -lasound rawmidi.c

