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
	@mkdir -p tests/output
	@rm -rf tests/output/*
	@echo Executing test1
	@valgrind --leak-check=full ./src/server/server ./tests/test1.conf > tests/output/server.log 2>tests/output/valgrind.log & echo "$$!" > tests/output/server.PID
	@sleep 2
	@bash tests/test1.sh
	@echo Sending SIGHUP to the server
	@kill -s 1 `cat tests/output/server.PID`
	@bash tests/wait `cat tests/output/server.PID`
	@echo Done
	@echo Total errors: `awk -F'SUMMARY: |errors' '/ERROR SUMMARY/{print $$2}' tests/output/valgrind.log`
	@echo Heap usage at exit: `awk -F': ' '/in use at exit/{print $$2}' tests/output/valgrind.log`

test2:
	@mkdir -p tests/output
	@rm -rf tests/output/*
	@echo Executing test2
	@valgrind --leak-check=full ./src/server/server ./tests/test2.conf > tests/output/server.log 2>tests/output/valgrind.log & echo "$$!" > tests/output/server.PID
	@sleep 2
	@bash tests/test2.sh
	@sleep 5
	@echo Sending SIGHUP to the server
	@kill -s 1 `cat tests/output/server.PID`
	@bash tests/wait `cat tests/output/server.PID`
	@echo Done
	@echo Total errors: `awk -F'SUMMARY: |errors' '/ERROR SUMMARY/{print $$2}' tests/output/valgrind.log`
	@echo Heap usage at exit: `awk -F': ' '/in use at exit/{print $$2}' tests/output/valgrind.log`