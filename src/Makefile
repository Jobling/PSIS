CC = gcc
CFLAGS = -Wall -g
DEPS = message.h psiskv_lib.h psiskv_database.h

%.o: %.c %.h
	$(CC) -c -o $@ $< $(CFLAGS)

server: psiskv_server.c psiskv_database.o $(DEPS)
	$(CC) -o server $^ $(CFLAGS)
	
test: psiskv_lib.o psiskv_database.o
	
clean:
	rm -f *.o
	rm -f server

