CC		      =  gcc
CFLAGS	      += -Wall -pedantic -std=c99
TARGETS	      = users matrix wc 
OPTFLAGS	  = -g -O3 
LIBS		  = -lpthread

BIN           = client

OBJECTS      := 

.PHONY: clean all
.SUFFIXES: .c .h

all: $(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) $(OPTFLAGS) $(LIBS) -o $(BIN)

%.o: %.c
	$(CC) $(CFLAGS) $(OPTFLAGS) -c $(LIBS) $< -o $@

clean : 
	-rm -rf $(OBJECTS) $(BIN)