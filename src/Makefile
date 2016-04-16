CC = gcc
CFLAGS = -Wall -g
DEPS = message.h psiskv_lib.h psiskv_database.h
LIBS = -pthread

%.o: %.c %.h
	$(CC) -c -o $@ $< $(CFLAGS)

server: psiskv_server.c psiskv_database.o $(DEPS)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

client: psiskv_client.c psiskv_lib.o
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	@rm -f server client *.o
	@echo "Done."
