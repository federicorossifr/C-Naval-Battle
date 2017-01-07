CC=gcc
CFLAGS=-Wall -m32
CLI=Client/clinterface
GME=Client/game
LIBF=build/lib
UTI=Utilities/utility.h
CLI_DEPS_F = $(LIBF)/utility.o $(LIBF)/cli.o $(LIBF)/board.o $(LIBF)/game.o $(LIBF)/client.o $(LIBF)/doers.o
SRV_DEPS_F = $(LIBF)/utility.o $(LIBF)/handlers.o $(LIBF)/user.o $(LIBF)/server.o
TARGETS= battle_client battle_server
$(LIBF)/utility.o: Utilities/utility.c Utilities/utility.h Utilities/server_codes.h Utilities/client_codes.h
	$(CC) -c -o $@ $< $(CFLAGS)

$(LIBF)/cli.o:  $(CLI)/cli.c $(CLI)/cli.h $(UTI)
	$(CC) -c -o $@ $< $(CFLAGS)

$(LIBF)/board.o: $(GME)/board.c $(GME)/board.h $(UTI)
	$(CC) -c -o $@ $< $(CFLAGS)

$(LIBF)/game.o: $(GME)/game.c $(UTI)
	$(CC) -c -o $@ $< $(CFLAGS)

$(LIBF)/doers.o: Client/doers.c $(UTI)
	$(CC) -c -o $@ $< $(CFLAGS)
	
$(LIBF)/client.o: Client/client.c $(LIBF)/doers.o $(UTI)
	$(CC) -c -o $@ $< $(CFLAGS)

$(LIBF)/handlers.o: Server/handlers.c $(UTI)
	$(CC) -c -o $@ $< $(CFLAGS)
	
$(LIBF)/user.o: Server/user.c Server/user.h $(UTI)
	$(CC) -c -o $@ $< $(CFLAGS)

$(LIBF)/server.o: Server/server.c $(LIBF)/handlers.o $(LIBF)/user.o
	$(CC) -c -o $@ $< $(CFLAGS)
	
battle_server: $(SRV_DEPS_F)
	$(CC) $(SRV_DEPS_F) -o $@ $(CFLAGS)
	
battle_client: $(CLI_DEPS_F)
	$(CC) $(CLI_DEPS_F) -o $@ $(CFLAGS)

all: $(TARGETS)	
	
clean:
	rm $(LIBF)/*
export:
	tar --exclude .directory  -cvzf src.tgz Client Server Utilities
