# stretch# apt install libasound-dev

IDIR = ../include/server
ODIR = obj

CC=gcc
CFLAGS=-I$(IDIR) -g
LIBS=-lasound -lpthread -lrt

all: midi-server ../pygame/app.pyc

DEPS = $(patsubst %.c,%.o,$(wildcard filters/*.h)) $(wildcard *.h)
OBJ = $(patsubst %.c,%.o,$(wildcard filters/*.c)) $(patsubst %.c,%.o,$(wildcard *.c))

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

../pygame/app.pyc: common.h app.h write.h thru.h macro.h
	cd ../pygame && python server.py && python -m compileall app.py && rm app.py

midi-server: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.phony: clean

clean:
	rm -f $(OBJ) midi-server
	rm ../pygame/app.py*
