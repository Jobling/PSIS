CC = gcc
CFLAGS = -Wall -g
LIBS = -pthread
EDITOR = geany
DEPS = message.h

CLIENT_OBJ = psiskv_lib.o comm_utils.o
FRONT_SERVER_OBJ = interprocess.o
DATA_SERVER_OBJ = interprocess.o psiskv_data_server_lib.o psiskv_database.o comm_utils.o

######### Make all targets ##########

all: clean objects client server

run: all
	@./front_server

######### Main objects for program usage #############
# client: program used for client side               #
# server: main server programs						 #
# front_server: front server program  				 #
# data_server: data server program (child of server) #
######################################################

client: psiskv_client.c $(CLIENT_OBJ) $(DEPS)
	$(CC) -o $@ $^ $(CFLAGS)

server: data_server front_server

front_server: psiskv_front_server.c $(FRONT_SERVER_OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

data_server: psiskv_data_server.c  $(DATA_SERVER_OBJ) $(DEPS)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

########## Development compiling ###############
# objects: compile all object files            #
# test: creates executables based on test code #
################################################

objects: $(CLIENT_OBJ) $(DATA_SERVER_OBJ) $(FRONT_SERVER_OBJ)

test: 

########## Wildcard compiling #############################
# %.o: compiles all binary (.o) files needed for programs #
# % compiles random testing programs                      #
###########################################################

%.o: %.c %.h
	@$(CC) -c -o $@ $< $(CFLAGS)

%: %.c psiskv_lib.o $(DEPS)
	$(CC) -o $@ $< $(CFLAGS) $(LIBS)

########## Auxiliary commands: ##########
# git_push: commits and pushes files    #
# edit: opens files to change project   #
# clean: cleans dependencies from make  #
#########################################

edit:
	@echo "Opening $(EDITOR)"
	@$(EDITOR) *.h *.c Makefile &

clean:
	@rm -f data_server client front_server *.o
	@echo "Cleaned."

git_push: clean
	@git commit -a
	@git push

# Rule declaring that other rules are not build targets (programs)

.PHONY: all run objects clean edit git_push client server front_server data_server
