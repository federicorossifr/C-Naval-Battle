#!/bin/bash
gcc -c -Wall Server/server.c
gcc -c -Wall Server/user.c
gcc -c -Wall Server/handlers.c
gcc -c -Wall Utilities/utility.c
gcc handlers.o server.o user.o utility.o -o build/server
./build/server 8080
