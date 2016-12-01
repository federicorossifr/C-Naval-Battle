#!/bin/bash
gcc -c -Wall Client/client.c
gcc -c -Wall Client/clinterface/cli.c
gcc -c -Wall Client/game/board.c
gcc -c -Wall Utilities/utility.c
gcc -c -Wall Client/doers.c
gcc -c -Wall Client/game/game.c 
gcc utility.o game.o client.o cli.o board.o doers.o -o build/client
./build/client 127.0.0.1 8080
