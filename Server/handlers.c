#include "../Utilities/utility.h"
#include "user.h"
extern user* head;
extern fd_set master;
void client_crashed(int socket);
void handle_log_in(int socket) {
    char* username; int port; int msgdim;
    boolean res;
    server_response sr;
    username=(char*)receiveMessage(socket,NULL,&msgdim);
    if(!username) return;
    
    res = recv_int(socket,NULL,&port);			
    if(!res) client_crashed(socket);
    user* found = search_by_username(head,username);
    if(!found)  {
        add_user(&head,port,username,socket);
        printf("[LOG] User %s correctly registered on %s:%d \n",username,get_ip_from_socket(socket),port);
        res = true;
    }
    else res = false;
    sr = (res)? USEROK:USERINVAL;
    if(!send_int(socket,NULL,sr)) client_crashed(socket);
    
}

void handle_disconnect(int socket,fd_set* mstr) {
    FD_CLR(socket,mstr);
    close(socket);
    user* disconnecting = search_by_fdset_index(head,socket);
    if(disconnecting == NULL) {
        printf("[LOG] Connection closed while registering\n");
        return;
    }
    server_response nak = CONN_REJ; 
    if(disconnecting->pending_conn_req_sock > 0) {
        user* other = search_by_fdset_index(head,disconnecting->pending_conn_req_sock);
        if(disconnecting->status == CONNECTING) {
            printf("[LOG] User was involved in connection, aborting connection\n");
            if(!send_int(disconnecting->pending_conn_req_sock,NULL,nak)) {
                client_crashed(disconnecting->pending_conn_req_sock);
                return;
            }
            if(other) other->status = FREE;
        } else if(disconnecting->status == PLAYING) {
            printf("[LOG] User was playing, signaling crashed match\n");
            if(!send_int(disconnecting->pending_conn_req_sock,NULL,MATCH_CRASHED)) {
                client_crashed(disconnecting->pending_conn_req_sock);
                return;
            }
            if(other) other->status = FREE;
        }
    }
    
    printf("[LOG] User %s disconnected\n",disconnecting->username);
    delete_user(&head,socket);
}

void client_crashed(int socket) {
    handle_disconnect(socket,&master);
}

void handle_who(int socket,int count) {
    user* walk = head;
    if(!send_int(socket,NULL,count)) return;
    
    while(walk != NULL) {
        if(socket != walk->fdset_index) {
            if(!sendMessage(socket,NULL,walk->username,strlen(walk->username)+1)) client_crashed(socket);
            if(!send_int(socket,NULL,walk->status)) client_crashed(socket);
        }
        walk = walk->next;
    }
}

void handle_conn_req(int socket) {
    int msg_dim;
    char* username =(char*)receiveMessage(socket,NULL,&msg_dim);
    user* requested = search_by_username(head,username); //user who receives request
    user* requesting = search_by_fdset_index(head,socket); //user who made reques
    server_response sr;
    printf("[LOG] New request from %s to %s\n",requesting->username,username);
    if(requested == NULL || requested->fdset_index == socket) {
        sr = NOUSER;
        printf("[LOG] User not found or self request\n");
        if(!send_int(socket,NULL,sr)) client_crashed(socket);
        return;
    }
    
    if(requested->status != FREE) {
        sr = BUSYUSER;
        printf("[LOG] User busy\n");
        if(!send_int(socket,NULL,sr)) client_crashed(socket);
        return;
    }
    
    printf("[LOG] Forwarding request to %s\n",username);
    requesting->status = CONNECTING;
    requested->status = CONNECTING;
    requested->pending_conn_req_sock = socket;
    requesting->pending_conn_req_sock = requested->fdset_index;
    
    client_request ur = CONN_REQ;
    if(!send_int(requested->fdset_index,NULL,ur)) client_crashed(requested->fdset_index);
    if(!sendMessage(requested->fdset_index,NULL,requesting->username,strlen(requesting->username)+1)) client_crashed(requested->fdset_index);    
}



void handle_conn_accept(int socket) {
    user* requested = search_by_fdset_index(head,socket);
    int requesting_socket = requested->pending_conn_req_sock;
    user* requesting = search_by_fdset_index(head,requesting_socket);
    if(!requesting) {
        return;
    }
    printf("[LOG] Sending connection ack from %s to %s\n",requested->username,requesting->username);
    server_response sr = CONN_OK;
    
    if(!send_int(requesting_socket,NULL,sr)) client_crashed(socket);
    
    printf("[LOG] Sending connection parameters to client %s\n", requesting->username);
    if(send(requesting_socket,(void*)requested->ip_addr,INET_ADDRSTRLEN+1,0) < INET_ADDRSTRLEN+1) {
        perror("[ERROR] Error sending IP to requested client");
        client_crashed(socket);
        return;
    };
    
    send_int(requesting_socket,NULL,requested->udp_port);
    
    
    send_int(socket,NULL,sr);
    printf("[LOG] Sending connection parameters to client %s\n", requested->username);
    if(send(socket,(void*)requesting->ip_addr,INET_ADDRSTRLEN+1,0) < INET_ADDRSTRLEN+1) {
        perror("[ERROR] Error sending IP to requesting client");
        client_crashed(socket);
        return;
    };
    
    if(!send_int(socket,NULL,requesting->udp_port)) client_crashed(socket);
    requesting->status = requested->status = BUSY;
    
}

void handle_conn_refuse(int socket) {
    user* requested = search_by_fdset_index(head,socket);
    requested->status = FREE;
    requested->pending_conn_req_sock = -1;    
    int requesting_socket = requested->pending_conn_req_sock;
    user* requesting = search_by_fdset_index(head,requesting_socket);
    if(!requesting) return;
    printf("[LOG] Sending connection nak from %s to %s",requested->username,requesting->username);
    server_response sr = CONN_REJ;
    requesting->status = FREE;
    requesting->pending_conn_req_sock = -1;
    if(!send_int(requesting_socket,NULL,sr)) client_crashed(socket);
    
}

void handle_ready(int socket) {
    user* ready = search_by_fdset_index(head,socket);
    int dual_sock = ready->pending_conn_req_sock;
    if(dual_sock < 0) return;
    user* dual = search_by_fdset_index(head,dual_sock);
    if(!dual) {
        send_int(socket,NULL,CONN_REJ);
        return;
    }
    server_response sr = MATCH_BEGIN;
    ready->status = PLAY_READY;
    printf("[LOG] User %s is ready to play.\n",ready->username);
    
    //BARRIER
    if(dual->status == PLAY_READY) {
        printf("[LOG] Enemy user %s is ready to play.\n",dual->username);
        if(!send_int(socket,NULL,sr)) {
            client_crashed(socket);
            return;
        }
        if(!send_int(dual_sock,NULL,sr)) {
            client_crashed(socket);
            return;
        }
        ready->status = dual->status = PLAYING;
    }
    
    if(dual->status == FREE) { //DUAL TIMED OUT
        if(!send_int(socket,NULL,MATCH_CRASHED)) {
            client_crashed(dual->fdset_index);
            return;
        }
    }
}

void handle_match_end(int socket) {
    user* user_socket = search_by_fdset_index(head,socket);
    printf("[LOG] User %s is now free\n",user_socket->username);
    user_socket->status = FREE;
    user_socket->pending_conn_req_sock = -1;
}