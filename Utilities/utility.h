#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <ctype.h>
#include "server_codes.h"
#include "client_codes.h"
#define STDIN 0
typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;
typedef enum boolean{false,true} boolean;

void setupAddress(sockaddr_in* ind,int port,char* addr);
int connectSocket(int sd,sockaddr_in* ind);
int setupServerSocket(sockaddr_in* ind);
int setupSocket(sockaddr_in* ind,int tipo);

boolean send_int(int ds,sockaddr_in* dst,int d);
boolean recv_int(int ds,sockaddr_in* src,int* d);


////CHANGE////
void* receiveMessage(int ds,sockaddr_in* src,int* msgdim);
void sendMessage(int ds,sockaddr_in* dst,char* message,int length);
//////////////

char* get_ip_from_socket(int socket);