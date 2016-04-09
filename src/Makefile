CC = gcc
CFLAGS = -Wall 
DEPS = message.h psiskv_lib.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

server: psiskv_server.o
	$(CC) -o server $^ $(CFLAGS)
	
clean:
	rm -f *.o

