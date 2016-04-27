CC = gcc
CFLAGS = -Wall -g
DEPS = message.h
LIBS = -pthread

all: clean client server 

test: prof-client-1 prof-client-2 test-client

server: psiskv_server.c psiskv_database.o psiskv_server_lib.o $(DEPS)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

client: psiskv_client.c psiskv_lib.o $(DEPS)
	$(CC) -o $@ $^ $(CFLAGS)
	
prof-client-1: cli-exe-1.c psiskv_lib.o $(DEPS)
	$(CC) -o $@ $^ $(CFLAGS)
	
prof-client-2: cli-exe-par-1.c psiskv_lib.o $(DEPS)
	$(CC) -o $@ $^ $(CFLAGS)
	
test-client: cli-exe-sync.c psiskv_lib.o $(DEPS)
	$(CC) -o $@ $^ $(CFLAGS)
	
%.o: %.c %.h
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	@rm -f server client test-client prof* *.o
	@echo "Cleaned."

.PHONY: clean
