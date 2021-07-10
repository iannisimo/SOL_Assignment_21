SERVER = src/server
CLIENT = src/client

.PHONY: test1 clean server client all
.SUFFIXES: .c .h

all: server client

server:
	+$(MAKE) -C $(SERVER)

client:
	+$(MAKE) -C $(CLIENT)

clean: 
	@echo I am cleaning all your mess... maybe
	+$(MAKE) clean -C $(SERVER)
	+$(MAKE) clean -C $(CLIENT)

test1:
	@rm -rf tests/output/*
	@echo Executing test1
	@valgrind --leak-check=full ./src/server/server ./tests/test1.conf > tests/output/valgrind.log 2>&1 & echo "$$!" > tests/output/server.PID
	@sleep 2
	@bash tests/test1.sh
	@echo Sending SIGHUP to the server
	@kill -s 1 `cat tests/output/server.PID`
	@echo Done


