CC		      =  gcc
CFLAGS	      += -Wall -pedantic -std=gnu11
TARGETS	      = users matrix wc 
OPTFLAGS	  = -g -O3 
LIBS		  = -lpthread

BIN           = server

OBJECTS      := connection.o test.o

.PHONY: clean all
.SUFFIXES: .c .h

all: clean $(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) $(OPTFLAGS) $(LIBS) -o $(BIN)

%.o: %.c
	$(CC) $(CFLAGS) $(OPTFLAGS) -c $(LIBS) $< -o $@

clean : 
	-rm -rf $(OBJECTS) $(BIN)