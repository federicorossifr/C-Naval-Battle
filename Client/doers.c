#include "../Utilities/utility.h"
#include "clinterface/cli.h"

extern int server_sock;
extern int game_socket;
void game_setup();
void do_udp_setup(int port) {
	sockaddr_in udpGameAddress;
	setupAddress(&udpGameAddress,port,NULL);
	game_socket = setupSocket(&udpGameAddress,SOCK_DGRAM);
}

void do_log_in() {
	client_request log_req = LOG_IN;
	server_response log_res;
	char* user_buff; int port;

	do {
		printff("Enter your username: ");
		scanf("%ms",&user_buff);
		printff("Enter your port: ");
		scanf("%d",&port);
		send_int(server_sock,NULL,log_req);

		sendMessage(server_sock,NULL,user_buff,strlen(user_buff)+1);

		send_int(server_sock,NULL,port);

		recv_int(server_sock,NULL,(int*)&log_res);

		if(log_res != USEROK) printf("ERROR username already exists\n");
	} while(log_res != USEROK);
	do_udp_setup(port);
	printf("OK registration successful!\n");
}

void do_disconnect() {
	client_request quit_req = QUIT;
	send_int(server_sock,NULL,quit_req);
	printf("Bye!\n");

	//also close udp socket
	close(server_sock);
	exit(0);
}

void do_who() {
	int msglen;
	client_request who_req = WHO;
	send_int(server_sock,NULL,who_req);

	int num_users;
	recv_int(server_sock,NULL,&num_users);

	char* tmpName;
	user_status s;
	int i;
	for(i=0; i < num_users-1; ++i) {
		tmpName = receiveMessage(server_sock,NULL,&msglen);

		recv_int(server_sock,NULL,(int*)&s);

		printf("%s(%s) \n",tmpName,get_status(s));
	}
}

void do_client_conn_req(char* username) {
	client_request conn_req = CONN_REQ;
	server_response conn_res;
	printf("[LOG]Sending connection request...\n");
	send_int(server_sock,NULL,conn_req);

	sendMessage(server_sock,NULL,username,strlen(username)+1);

	recv_int(server_sock,NULL,(int*)&conn_res);
	switch(conn_res) {
		case NOUSER: printf("No user with name %s\n",username); break;
		case BUSYUSER: printf("User %s is busy\n",username); break;
		case CONN_REJ: printf("User %s has rejected your request\n",username); break;
		case CONN_OK: printf("User %s accepted your invite\n[LOG] Loading game setup...!\n",username);
					  game_setup();break;
		default: break;
	}
}

void do_send_invite_res(client_request res) {
	send_int(server_sock,NULL,res);
	
}

