#include "../../Utilities/utility.h"
#include "../clinterface/cli.h"
#include "board.h"
typedef enum turn {
	YOU,ENEMY
} turn;
typedef enum game_state {
	ENEMY_FIRING,ENEMY_FIRING_F,ALLY_FIRING,ALLY_IDLE,ENEMY_IDLE,ALLY_WAIT
} game_state;

sockaddr_in enemy_addr;
extern int game_socket;
extern int server_sock;
extern menu_voice GAME_MENU[];
extern const int GAME_COUNT;
extern board_cell_status allies[BOARD_SIZE][BOARD_SIZE];
extern board_cell_status enemies[BOARD_SIZE][BOARD_SIZE];

void game();
void terminate_match() {
	client_request end_req = MATCH_END;
	send_int(server_sock,NULL,end_req);
}

void ask_place() {
	int i; char row; int col;
	printf("Place your battleships\n");
	printf("%d\n",BATTLESHIPS_NUMBER);
	for(i = 0; i < BATTLESHIPS_NUMBER;++i) {
		printf("Number %d: ",i+1);
		fflush(stdout);
		discard();
		scanf("%1c%1d",&row,&col);
		if(place_battelship(row,col)) {
			printf("Inserted battleship at %c%d\n",row,col);
			continue;
		}
		printf("Error placing battleship in %c%d.\n",row,col);
		i--;
	}
}

boolean synchronize() {
	printf("[LOG]Sending ready ack to server\n");
	client_request ready_req = READY;
	server_response sr;
	if(!send_int(server_sock,NULL,ready_req)) return false;

	printf("[LOG]Waiting for synchcronize signal\n");

	fd_set master; FD_ZERO(&master);
	FD_SET(server_sock,&master);
	struct timeval timeout = {60,0};
	select(server_sock+1,&master,NULL,NULL,&timeout); // TIMER
	if(FD_ISSET(server_sock,&master)) {
		if(!recv_int(server_sock,NULL,(int*)&sr)) return false;

		return sr == MATCH_BEGIN;
	} else {
		printf("Synchronize signal wait timed out, terminating\n");
		return false;
	}
	
	//can reach this point?
	return false;
}

void game_setup(int r) {
	char* ip = malloc(INET_ADDRSTRLEN+1);
	int udp_port;
  	printf("[LOG]User accepted request\n");


	if(recv(server_sock,(void*)ip,INET_ADDRSTRLEN+1,0) < INET_ADDRSTRLEN+1) {
	  terminate_match();
	  return;
	}

	if(!recv_int(server_sock,NULL,(int*)&udp_port)) {
	  terminate_match();
	  return;
	}


	setupAddress(&enemy_addr,udp_port,ip);
	game_state t = (r==0)?ALLY_IDLE:ENEMY_IDLE;
	printf("[LOG]Ready to play\n");
	boards_initialize();
	ask_place();
	if(!synchronize()) {
	  printf("[ERROR] Failed to synchronize clients, leaving game.\n");
	  terminate_match();
	  return;
	}
	game(t);

}

int handle_enemy_fire(char row,int col) {
	board_cell_status res;
	in_game_message result;
	in_game_message in_case_we_lose = YOU_WIN;
	int rem;
	res = try_hit(row,col,&rem);
	if(res != HIT) {
		printf("Our enemy missed! ;)\n");
		result = SHIP_MISSED;
	} else {
		printf("Our enemy hit our ship!!! MAYDAY!! :O\n");
		result = SHIP_HIT;
	}
	printf("Remaining: %d\n\n",rem);

	boolean ress = send_int(game_socket,&enemy_addr,result);
	printf("Sended message %d\n",ress);
	if(rem == 0) {
		printf("It was our last ship, GAME OVER </3\n");
		send_int(game_socket,&enemy_addr,in_case_we_lose);
	}
	return rem;
}

void fire(char row,int col) {
	printf("\nFire on %c%d\n",row,col);
	in_game_message fire_msg = FIRE;
	send_int(game_socket,&enemy_addr,fire_msg);
	sendto(game_socket,(void*)&row,sizeof(char),0,(sockaddr*)&enemy_addr,sizeof(enemy_addr));
	send_int(game_socket,&enemy_addr,col);
	printf("Fired!\n");
}

void surrend() {
	printf("You have surrended! Maybe next time :P\n");
	in_game_message surr_msg = SURR;
	send_int(game_socket,&enemy_addr,surr_msg);
}

void demux_mesage(in_game_message msg,char row,int col) {
	//sockaddr_in sndr; unsigned int len = sizeof(sndr);
	printf("[LOG]Received %d\n",msg);
	switch(msg) {
		case SHIP_HIT:
			printf("Hit!\n"); 
			mark_board(row,col,HIT); 
			break;
		case SHIP_MISSED:
			printf("Missed!\n"); 
			mark_board(row,col,MISSED); 
			break;
		case SURR:
			printf("The enemy surrended\n");
		case YOU_WIN:
			printf("We won this battle, but not the war!\n");
			break;
		default: break;
	}
}

boolean parse_command(game_state* state,char* r,int* c) {
	char* cmd; int cmdkey;
	scanf("%ms",&cmd);
	cmdkey = get_menu_key(cmd,GAME_MENU,GAME_COUNT);
	switch(cmdkey) {
		case 1:  //help
			print_help(GAME_MENU,GAME_COUNT); break;
		case 2: //disconnect
			surrend();return true;break;
		case 3: //shot
			scanf("%1c",r); //garbage
			scanf("%1c%1d",r,c);
			*state = ALLY_WAIT;
			fire(*r,*c);
			printf("Waiting...\n");
			break;
		case 4: //show
			printf("\n***********************************\n");
			printf("**|| 0 ship alive | X ship hit | ^ missed shot | - sea ||**\n");
			printf("Your board:\n");
			print_board(allies);
			printf("Enemy's board:\n");
			print_board(enemies);
			printf("\n***********************************\n");			
			break;
	}
	discard();
	return false;
}




void game(game_state t) {
	game_state state = t;
	fd_set master; FD_ZERO(&master);
	fd_set read_ready; FD_ZERO(&read_ready);
	int fdmax = game_socket;
	FD_SET(game_socket,&master);
	FD_SET(STDIN,&master);
	int fdescriptor;
	char row;int col;
	struct timeval timeout = {60,0};
	boolean timedout = true;
	for(;;) {
		read_ready = master;
			switch(state) {
				case ALLY_IDLE: printf("Your turn %d\n",state);printff("#");break;
				case ENEMY_IDLE: printf("Enemy turn\n"); break;
				default:break;
			}
		select(fdmax+1,&read_ready,NULL,NULL,&timeout); // TIMER
		for(fdescriptor = 0; fdescriptor <= fdmax; ++fdescriptor) {
			if(FD_ISSET(fdescriptor,&read_ready)) {
				timedout = false;
				if(fdescriptor == game_socket) {
					printf("Message in socket\n");
					in_game_message msg; int msglen;
					sockaddr_in sndr; unsigned int len = sizeof(sndr);
					
					switch(state) {
						case ENEMY_IDLE:
							msglen = recv_int(game_socket,&sndr,(int*)&msg);
							if(msg == FIRE) state=ENEMY_FIRING;
							if(msg == SURR) printf("The enemy surrended\n");
							if(msg == YOU_WIN || msg == SURR) {
								printf("We won this battle, but not the war!\n"); 
								terminate_match();
								return;
							}
							break;
						case ENEMY_FIRING:
							msglen = recvfrom(game_socket,(void*)&row,sizeof(char),0,(sockaddr*)&sndr,&len);
							printf("Enemy fired row %c\n",row);
							state = ENEMY_FIRING_F;
							break;
						case ENEMY_FIRING_F:
							msglen = recv_int(game_socket,&sndr,(int*)&col);
							printf("Enemy fired col %d\n",col);
							if(!handle_enemy_fire(row,col)) {
								terminate_match();
								return;
							}
							state = ALLY_IDLE;
							break;
						case ALLY_WAIT:
							msglen = recv_int(game_socket,&enemy_addr,(int*)&msg);
							demux_mesage(msg,row,col);
							state = ENEMY_IDLE;
							break;
						default: break;
					}
					if(msglen == 0) {
						perror("recvfrom");
						exit(1);
					}
					continue;
				}
				//STDIN
				printf("Reading in STDIN\n");
				switch(state) {
					case ALLY_IDLE:
						if(parse_command(&state,&row,&col)) {
							terminate_match();
							return; 
						}
						break;
					default: discard(); break;
				}
			}
		}
		if(timedout) {
			printf("Timed out!\n");
			terminate_match();
			return;
		}
		else {
			timedout = true;
			timeout.tv_sec = 60;
		}
	}
}