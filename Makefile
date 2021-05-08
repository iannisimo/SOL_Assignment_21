SERVER = src/server
CLIENT = src/client

.PHONY: clean server client all
.SUFFIXES: .c .h

all: server client

server:
	+$(MAKE) -C $(SERVER)

client:
	+$(MAKE) -C $(CLIENT)

clean : 
	@echo I am cleaning all your mess... maybe
	+$(MAKE) clean -C $(SERVER)
	+$(MAKE) clean -C $(CLIENT)