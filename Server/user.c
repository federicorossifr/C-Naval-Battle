#include "../Utilities/utility.h"
#include "user.h"
extern int counter;
user* search_by_username(user* head,char* username) {
    user* walk = head;
    if(!username) return NULL;
    while(walk!= NULL) {
        if(strcmp(walk->username,username) == 0) return walk;
        
        walk = walk->next;
    }
    return NULL;
}

user* search_by_fdset_index(user* head,int fdset_index) {
    user* walk = head;
    if(fdset_index < 0) return NULL;
    while(walk!=NULL) {
        if(walk->fdset_index == fdset_index) return walk;
        walk = walk->next;
    }
    return NULL;
}




void add_user(user** head,int port,char* username,int fdset_index) {
    if(username == NULL || fdset_index < 0) return;
    user* tmp = malloc(sizeof(user));
    tmp->username = malloc(strlen(username)+1);
    strcpy(tmp->username,username);
    tmp->fdset_index = fdset_index;
    tmp->udp_port = port;
    tmp->status = FREE;
    tmp->pending_conn_req_sock = -1;
    tmp->next = *head;
    tmp->ip_addr = get_ip_from_socket(fdset_index);
    *head = tmp;
    counter++;
}

void delete_user(user** head,int fdset_index) {
    user* walk = *head;
    user* prev = NULL;
    for(;walk != NULL && walk->fdset_index != fdset_index; walk = walk->next)
        prev = walk;
    
    //not found
    if(walk == NULL) return;
    
    counter--;
    //found @ head
    if(walk == *head) {
        *head = (*head)->next;
        free(walk);
        return;
    }
    
    //found elsewhere
    prev->next = walk->next;
    free(walk);
}



//util
void print_list(user* head) {
    user* walk = head;
    printf("************************************\n");
    while(walk!=NULL) {
        printf("Username: %s\n",walk->username);
        printf("Port: %d\n",walk->udp_port);
        printf("Socket: %d\n",walk->fdset_index);
        printf("------------------------------------\n");
        walk = walk->next;
    }
    
    printf("************************************\n");
}
