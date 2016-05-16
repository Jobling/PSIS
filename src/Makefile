CC = gcc
CFLAGS = -Wall -g
LIBS = -pthread
EDITOR = geany

IDIR = ../include
ODIR = obj

##########################################
# $(patsubst pattern,replacement,$(var)) #
# $(var:pattern=rplacement)              #
##########################################

#### MAKING #####
_DEPS = message.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = psiskv_database.o psiskv_server_lib.o psiskv_lib.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

#### LOOK AT THIS #####

all: clean client data_server

######### Main objects for program usage #############
# client: program used for client side               #
# server: main server program                        #
# data_server: data server program (child of server) #
######################################################

client: psiskv_client.c $(OBJ) $(DEPS)
	$(CC) -o $@ $^ $(CFLAGS)

data_server: psiskv_data_server.c $(OBJ) $(DEPS)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

########## Development compiling ###############
# test: creates executables based on test code #
################################################

test: cli-exe-1 cli-exe-par-1 cli-exe-sync

########## Wildcard compiling #############################
# %.o: compiles all binary (.o) files needed for programs #
# % compiles random testing programs                      #
###########################################################

$(ODIR)/%.o: %.c $(IDIR)/%.h
	$(CC) -c -o $@ $< $(CFLAGS)

%: %.c $(ODIR)/psiskv_lib.o $(DEPS)
	$(CC) -o $@ $< $(CFLAGS)

########## Auxiliary commands: ##########
# edit: opens files to change project   #
# clean: cleans dependencies from make  #
#########################################

edit:
	$(EDITOR) $(IDIR)/* psiskv*.c Makefile &

clean:
	@rm -f data_server client cli-exe-* $(ODIR)/*.o
	@echo "Cleaned."

# Rule declaring that <clean> and <edit> are not build targets (programs)

.PHONY: clean edit
