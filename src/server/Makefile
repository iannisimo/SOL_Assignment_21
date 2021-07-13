CC		      =  gcc
CFLAGS	      += -Wall -pedantic -std=gnu11 -MD
TARGETS	      = users matrix wc 
OPTFLAGS	  = -g -O3 
LIBS		  = -lpthread

BUILD		  = ../../build/server/
EXE           = ../../exe/

BIN           = $(EXE)server

OBJECTS      := utils.o config.o worker.o queue.o storage.o connection.o server.o
OBJECTS      := $(addprefix $(BUILD),$(OBJECTS))

.PHONY: clean all
.SUFFIXES: .c .h

all: prepbuild $(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) $(OPTFLAGS) $(LIBS) -o $(BIN)

prepbuild:
	@mkdir -p $(EXE)
	@mkdir -p $(BUILD)

$(BUILD)%.o: %.c
	$(CC) $(CFLAGS) $(OPTFLAGS) -c $(LIBS) $< -o $@

clean :
	-rm -rf $(OBJECTS) $(BIN)

-include $(OBJECTS:.o=.d)