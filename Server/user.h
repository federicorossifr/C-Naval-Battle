typedef struct user {
	char* username; //
	int fdset_index; //
	int udp_port; //
	char* ip_addr;
	user_status status;
	int pending_conn_req_sock;
	struct user* next;
} user;



user* search_by_username(user* head,char* username);
user* search_by_fdset_index(user* head,int fdset_index);
void  add_user(user** head,int port,char* username,int fdset_index);
void  delete_user(user** head,int fdset_index);


//util
void print_list(user* head);
