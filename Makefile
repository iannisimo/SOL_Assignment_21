SERVER = src/server
CLIENT = src/client

.PHONY: test1 test2 clean server client all
.SUFFIXES: .c .h

all: server client

server:
	+$(MAKE) -C $(SERVER)

client:
	+$(MAKE) -C $(CLIENT)

clean: 
	@rm -rf tests/output/*
	+$(MAKE) clean -C $(SERVER)
	+$(MAKE) clean -C $(CLIENT)

test1:
	@mkdir -p tests/output/test1
	@rm -rf tests/output/test1/*
	@echo Executing test1
	@valgrind --leak-check=full ./exe/server ./tests/test1.conf > tests/output/test1/server.log 2>tests/output/test1/valgrind.log & echo "$$!" > tests/output/test1/server.PID
	@sleep 2
	@bash tests/test1.sh
	@echo Sending SIGHUP to the server
	@kill -s 1 `cat tests/output/test1/server.PID`
	@bash tests/wait `cat tests/output/test1/server.PID`
	@rm tests/output/test1/server.PID
	@echo Done
	@bash tests/check.sh test1

test2:
	@mkdir -p tests/output/test2
	@rm -rf tests/output/test2/*
	@echo Executing test2
	@valgrind --leak-check=full ./exe/server ./tests/test2.conf > tests/output/test2/server.log 2>tests/output/test2/valgrind.log & echo "$$!" > tests/output/test2/server.PID
	@sleep 2
	@bash tests/test2.sh
	@sleep 5
	@echo Sending SIGHUP to the server
	@kill -s 1 `cat tests/output/test2/server.PID`
	@bash tests/wait `cat tests/output/test2/server.PID`
	@rm tests/output/test2/server.PID
	@echo Done
	@bash tests/check.sh test2

check:
	@echo `bash tests/check.sh test1 sum` `bash tests/check.sh test2 sum` | awk -v sum=0 '{for (i=1;i<=NF;i++) {sum += $$i}} END {if (sum == 0) {print "PASSED"; exit 0} else {print "NOT PASSED: " err " ERRORS" ;exit 1;}}'
