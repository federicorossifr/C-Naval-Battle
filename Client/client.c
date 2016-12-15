#include "../Utilities/utility.h"
#include "clinterface/cli.h"
extern const int MAIN_COUNT;
extern menu_voice MAIN_MENU[];
int server_sock; 
int game_socket;
char prompt = '>';

void do_log_in();
void do_disconnect(void);
void do_who(void);
void do_client_conn_req(char*);
void do_send_invite_res(client_request res);
void handle_client_conn_req();
void game_setup(int whostart);

void consume_user_input() {
    char* cmd; int cmdkey;
    scanf("%ms",&cmd);
    cmdkey = get_menu_key(cmd,MAIN_MENU,MAIN_COUNT);
    switch(cmdkey) {
        case 1:  //help
            print_help(MAIN_MENU,MAIN_COUNT); break;
        case 2: //who
            do_who(); break;
        case 3: //quit
            do_disconnect();
            break;
        case 4: //connect
            scanf("%ms",&cmd);
            printf("Connecting to %s\n",cmd);
            do_client_conn_req(cmd); 
            return;
    }
    discard();
}

typedef enum client_state {
    CLIENT_IDLE,CLIENT_CONN_REQ,CLIENT_CONN_REQ1,CLIENT_CONN_REQ2
} client_state;

int main(int argc,char* argv[]) {
    if(argc < 3) {
        printf("[ERROR] Usage: ./battle_client <server-ip> <server-port>\n");
        exit(-1);
    }
    client_state state = CLIENT_IDLE;
    int s_port = atoi(argv[2]);
    int msglen; //int status;
    sockaddr_in srv_addr;
    setupAddress(&srv_addr,s_port,argv[1]);
    server_sock = setupSocket(NULL,SOCK_STREAM);
    connectSocket(server_sock,&srv_addr);
    printf("Connected to server %s:%d\n",argv[1],s_port);
    /***log in***/
    do_log_in();
    /************/
    
    /***setting up fd_set for select***/
    fd_set master; FD_ZERO(&master);
    fd_set read_ready; FD_ZERO(&read_ready);
    int fdmax = server_sock;
    FD_SET(server_sock,&master);
    FD_SET(STDIN,&master);
    int fdescriptor; 
    
    //timeout for connection request prompt
    struct timeval timeout = {10,0}; 
    /**********************************/
    
    boolean timedout = true;
    char* username; char user_choose;
    for(;;) {
        switch(state) {
            case CLIENT_IDLE: printff(&prompt); break;
            default:break;
        }
        read_ready = master;
        //check if in connection prompt state and activate the timer
        if(state == CLIENT_CONN_REQ1) {
            select(fdmax+1,&read_ready,NULL,NULL,&timeout); // TIMER??
        }
        else {
            select(fdmax+1,&read_ready,NULL,NULL,NULL); // TIMER??
        }
        for(fdescriptor = 0; fdescriptor <= fdmax; ++fdescriptor) {
            if(FD_ISSET(fdescriptor,&read_ready)) {
                timedout = false;
                if(fdescriptor == server_sock) {
                    client_request cn; server_response sr;
                    //server-handling FSM
                    switch(state) {
                        case CLIENT_IDLE:
                            msglen = recv_int(fdescriptor,NULL,(int*)&cn);
                            if(cn == CONN_REQ) {
                                state = CLIENT_CONN_REQ;
                            }
                            break;
                        case CLIENT_CONN_REQ:
                            username = receiveMessage(server_sock,NULL,&msglen);
                            printf("\nClient %s wants to play. Accept(y/n)?",username);
                            fflush(stdout);
                            state = CLIENT_CONN_REQ1;
                            break;
                        case CLIENT_CONN_REQ2:
                            msglen = recv_int(server_sock,NULL,(int*)&sr);
                            if(sr == CONN_OK) {
                                game_setup(1);
                            } else {
                                printf("\nClient prematurely shutted down\n");
                            }
                            state = CLIENT_IDLE;
                            continue;
                        default: break;
                    }
                    if(msglen <=0) {
                        printf("\nServer gone down\n");
                        close(server_sock);
                        return 0;
                    }
                    continue;
                }
                //STDIN
                if(state == CLIENT_CONN_REQ1) {
                    fflush(stdout);
                    user_choose = getchar(); //discard();
                    user_choose = tolower(user_choose);
                    if(user_choose == 'y') {
                        printf("Match is starting\n");
                        do_send_invite_res(CONN_ACK);
                        state = CLIENT_CONN_REQ2;
                    } else if(user_choose == 'n') {
                        do_send_invite_res(CONN_NAK);
                        state = CLIENT_IDLE;
                    } 
                } else {
                    consume_user_input();
                }
            }
        }
        if(timedout && state == CLIENT_CONN_REQ1) {
            printf("Timedout request!\n");
            do_send_invite_res(CONN_NAK);
            state = CLIENT_IDLE;
            timedout = false;
        } else {
            timedout = true;
        }
        timeout.tv_sec = 10;
    }
    close(server_sock);
    return 0;
}