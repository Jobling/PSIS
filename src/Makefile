CC = gcc
CFLAGS = -Wall 
DEPS = message.h psiskv_lib.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

server: psiskv_server.c $(DEPS)
	$(CC) -o server $^ $(CFLAGS)
	
test: psiskv_lib.o
	
clean:
	rm -f *.o
	rm -f server

