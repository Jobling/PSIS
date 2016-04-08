CC = gcc
CFLAGS = -Wall -ansi
DEPS = message.h psiskv.h

%.o: %.c $(DEPS)
    $(CC) -c -o $@ $< $(CFLAGS)
    
server: psiskv_server.o
    $(CC) -o server $^ $(CFLAGS)

