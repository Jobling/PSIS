CC = gcc
CFLAGS = -Wall -g
LIBS = -pthread
EDITOR = geany

CLIENT_OBJ = psiskv_lib.o
SERVER_OBJ = psiskv_server.o psiskv_database.o

######### Make all targets ##########

all: clean client data_server

######### Main objects for program usage #############
# client: program used for client side               #
# server: main server program                        #
# data_server: data server program (child of server) #
######################################################

client: psiskv_client.c $(CLIENT_OBJ) $(DEPS)
	$(CC) -o $@ $^ $(CFLAGS)

data_server: psiskv_data_server.c  $(SERVER_OBJ) $(DEPS)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

########## Development compiling ###############
# test: creates executables based on test code #
################################################

test: cli-exe-1 cli-exe-par-1 cli-exe-sync

########## Wildcard compiling #############################
# %.o: compiles all binary (.o) files needed for programs #
# % compiles random testing programs                      #
###########################################################

%.o: %.c %.h
	$(CC) -c -o $@ $< $(CFLAGS)

%: %.c psiskv_lib.o $(DEPS)
	$(CC) -o $@ $< $(CFLAGS)

########## Auxiliary commands: ##########
# edit: opens files to change project   #
# clean: cleans dependencies from make  #
#########################################

edit:
	$(EDITOR) *.h psiskv*.c Makefile &

clean:
	@rm -f data_server client *.o
	@echo "Cleaned."

# Rule declaring that <clean> and <edit> are not build targets (programs)

.PHONY: all clean edit
