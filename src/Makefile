CC = gcc
CFLAGS = -Wall -g
LIBS = -pthread
EDITOR = geany

CLIENT_OBJ = psiskv_lib.o comm_utils.o
DATA_SERVER_OBJ = psiskv_data_server_lib.o psiskv_database.o comm_utils.o

######### Make all targets ##########

all: clean client data_server

######### Main objects for program usage #############
# client: program used for client side               #
# server: main server program                        #
# data_server: data server program (child of server) #
######################################################

client: psiskv_client.c $(CLIENT_OBJ) $(DEPS)
	$(CC) -o $@ $^ $(CFLAGS)

data_server: psiskv_data_server.c  $(DATA_SERVER_OBJ) $(DEPS)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

########## Development compiling ###############
# objects: compile all object files            #
# test: creates executables based on test code #
################################################

objects: $(CLIENT_OBJ) $(DATA_SERVER_OBJ)

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
# git_push: commits and pushes files    #
# edit: opens files to change project   #
# clean: cleans dependencies from make  #
#########################################	

edit:
	@echo "Opening $(EDITOR)"
	@$(EDITOR) *.h psiskv*.c comm*.c Makefile &

clean:
	@rm -f data_server client *.o
	@echo "Cleaned."

git_push: clean
	@git commit -a
	@git push

# Rule declaring that <clean> and <edit> are not build targets (programs)

.PHONY: all objects clean edit git_push
