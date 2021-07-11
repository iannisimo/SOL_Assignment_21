CC		      =  gcc
CFLAGS	      += -Wall -pedantic -std=gnu11
TARGETS	      = users matrix wc 
OPTFLAGS	  = -g -O3 
LIBS		  = -lpthread

BUILD		  = ../../build/server/
EXE           = ../../exe/

BIN           = $(EXE)server

WITHHEADER   := utils.o config.o worker.o queue.o storage.o connection.o
OBJECTS      := $(WITHHEADER) server.o
HEADERS      := $(addsuffix .h,$(basename $(WITHHEADER)))

.PHONY: clean all
.SUFFIXES: .c .h

all: prepbuild $(addprefix $(BUILD),$(OBJECTS)) $(HEADERS)
	$(CC) $(addprefix $(BUILD),$(OBJECTS)) $(CFLAGS) $(OPTFLAGS) $(LIBS) -o $(BIN)

prepbuild:
	@mkdir -p $(EXE)
	@mkdir -p $(BUILD)

$(addprefix $(BUILD),%.o): %.c
	$(CC) $(CFLAGS) $(OPTFLAGS) -c $(LIBS) $< -o $@

clean : prepbuild
	-rm -rf $(OBJECTS) $(BIN)