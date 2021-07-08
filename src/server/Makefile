CC		      =  gcc
CFLAGS	      += -Wall -pedantic -std=gnu11
TARGETS	      = users matrix wc 
OPTFLAGS	  = -g -O3 
LIBS		  = -lpthread

BIN           = server

OBJECTS      := server.o utils.o config.o worker.o queue.o storage.o connection.o

.PHONY: clean all
.SUFFIXES: .c .h

all: clean $(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) $(OPTFLAGS) $(LIBS) -o $(BIN)

%.o: %.c
	$(CC) $(CFLAGS) $(OPTFLAGS) -c $(LIBS) $< -o $@

clean : 
	-rm -rf $(OBJECTS) $(BIN)