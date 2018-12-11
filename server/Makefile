
IDIR = ../include/server
ODIR = obj

CC=gcc
CFLAGS=-I$(IDIR)
LIBS=-lasound -lpthread

all: midi-server

DEPS = thru.h write.h main.h
OBJ = thru.o write.o main.o

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

midi-server: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.phony: clean

clean:
	rm -f *.o midi-server
