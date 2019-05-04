# stretch# apt install libasound-dev

IDIR = ../include/server
ODIR = obj

CC=gcc
CFLAGS=-I$(IDIR) -g
LIBS=-lasound -lpthread

all: midi-server

DEPS = macro.h socket.h list.h thru.h write.h app.h main.h
OBJ = macro.o socket.o list.o thru.o write.o app.o main.o

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

midi-server: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.phony: clean

clean:
	rm -f *.o midi-server
