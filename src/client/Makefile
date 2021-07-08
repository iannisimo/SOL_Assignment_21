CC		      =  gcc
CFLAGS	     += -Wall -pedantic -std=gnu11
TARGETS	      = users matrix wc 
# OPTFLAGS	  = -g -O3 
OPTFLAGS	  = -g -O0

AR            = ar
ARFLAGS       = rvs

BIN           = client

HEADERS      := const.h

LIBS         := connectionAPI.a
OBJECTS      := utils.o argparse.o client.o

.PHONY: clean all
.SUFFIXES: .c .h

all: clean $(OBJECTS) $(HEADERS) $(LIBS)
	$(CC) $(OBJECTS) $(LIBS) $(CFLAGS) $(OPTFLAGS) -o $(BIN)

%.a: %.o %.h
	$(AR) $(ARFLAGS) $@ $<

%.o: %.c
	$(CC) $(CFLAGS) $(OPTFLAGS) -c $< -o $@


clean : 
	-rm -rf $(OBJECTS) $(BIN) $(LIBS)