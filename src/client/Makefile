CC		      =  gcc
CFLAGS	     += -Wall -pedantic -std=gnu11 -MD
TARGETS	      = users matrix wc 
OPTFLAGS	  = -g -O3

AR            = ar
ARFLAGS       = rvs

BUILD		  = ../../build/client/
EXE           = ../../exe/

BIN           = $(EXE)client

LIBS         := connectionAPI.a
LIBS         := $(addprefix $(BUILD), $(LIBS))
OBJECTS      := utils.o argparse.o client.o
OBJECTS      := $(addprefix $(BUILD), $(OBJECTS))

.PHONY: prepbuild clean all
.SUFFIXES: .c .h

all: prepbuild $(OBJECTS) $(LIBS)
	$(CC) $(OBJECTS) $(LIBS) $(CFLAGS) $(OPTFLAGS) -o $(BIN)

prepbuild:
	@mkdir -p $(EXE)
	@mkdir -p $(BUILD)

$(BUILD)%.a: $(BUILD)%.o
	$(AR) $(ARFLAGS) $@ $<

$(BUILD)%.o: %.c
	$(CC) $(CFLAGS) $(OPTFLAGS) -c $< -o $@

clean : 
	-rm -rf $(OBJECTS) $(LIBS) $(BIN)

-include $(OBJECTS:.o=.d)
-include $(LIBS:.a=.d)