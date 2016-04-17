CC = gcc
CFLAGS = -Wall -g
DEPS = message.h
LIBS = -pthread

all: clean client server

server: psiskv_server.c psiskv_database.o $(DEPS)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

client: psiskv_client.c psiskv_lib.o
	$(CC) -o $@ $^ $(CFLAGS)

%.o: %.c %.h
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	@rm -f server client *.o
	@echo "Cleaned."

.PHONY: clean
