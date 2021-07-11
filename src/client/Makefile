CC		      =  gcc
CFLAGS	     += -Wall -pedantic -std=gnu11
TARGETS	      = users matrix wc 
OPTFLAGS	  = -g -O3

AR            = ar
ARFLAGS       = rvs

BUILD		  = ../../build/client/
EXE           = ../../exe/

BIN           = $(EXE)client


LIBS         := connectionAPI.a
WITHHEADER   := utils.o argparse.o
OBJECTS      := $(WITHHEADER) client.o
HEADERS      := $(addsuffix .h,$(basename $(WITHHEADER) $(LIBS)))

.PHONY: prepbuild clean all
.SUFFIXES: .c .h

all: prepbuild $(addprefix $(BUILD),$(OBJECTS) $(LIBS)) $(HEADERS)
	$(CC) $(addprefix $(BUILD),$(OBJECTS) $(LIBS)) $(CFLAGS) $(OPTFLAGS) -o $(BIN)

prepbuild:
	@mkdir -p $(EXE)
	@mkdir -p $(BUILD)

$(addprefix $(BUILD),%.a): $(addprefix $(BUILD),%.o)
	$(AR) $(ARFLAGS) $@ $<

$(addprefix $(BUILD),%.o): %.c
	$(CC) $(CFLAGS) $(OPTFLAGS) -c $< -o $@

%.a: %.o
	$(AR) $(ARFLAGS) $@ $<

%.o: %.c
	$(CC) $(CFLAGS) $(OPTFLAGS) -c $< -o $@


clean : 
	-rm -rf $(addprefix $(BUILD),$(OBJECTS) $(LIBS)) $(BIN)