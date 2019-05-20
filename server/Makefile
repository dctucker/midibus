# stretch# apt install libasound-dev

IDIR = ../include/server
ODIR = obj

CC=gcc
CFLAGS=-I$(IDIR) -g
LIBS=-lasound -lpthread -lrt

all: midi-server

DEPS = $(patsubst %.c,%.o,$(wildcard filters/*.h)) $(wildcard *.h)
OBJ = $(patsubst %.c,%.o,$(wildcard filters/*.c)) $(patsubst %.c,%.o,$(wildcard *.c))

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

midi-server: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.phony: clean

clean:
	rm -f $(OBJ) midi-server
