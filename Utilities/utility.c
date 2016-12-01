#include "utility.h"
void setupAddress(sockaddr_in* ind,int port,char* addr) {
	memset(ind,0,sizeof(*ind));
	ind->sin_port = htons(port);
	ind->sin_family = AF_INET;
	int status;
	if(addr != NULL) {
		status = inet_pton(AF_INET,addr,&ind->sin_addr);
		if(status <= 0) {
			perror("inet_pton");
			exit(1);
		}
	}
	else
		ind->sin_addr.s_addr = INADDR_ANY;
}

int connectSocket(int sd,sockaddr_in* ind) {
	int status;
	status = connect(sd,(struct sockaddr*)ind,sizeof(*ind));
	if(status < 0) {
		perror("connect");
		exit(1);
	}
	return status;
}

int setupServerSocket(sockaddr_in* ind) {
	int status;
	int sd = setupSocket(ind,SOCK_STREAM);
	status=listen(sd,10);
	if(status < 0) {
		perror("listen");
		exit(1);
	}
	return sd;
}

int setupSocket(sockaddr_in* ind,int type) {
	int d = socket(AF_INET,type,0);
	int status;

	if(d < 0) {
		perror("socket");
		exit(1);
	}

	if(ind == NULL) return d;
		status = bind(d,(struct sockaddr*)ind,sizeof(*ind));
	if(status < 0) {
		perror("bind");
		exit(1);
	}
	return d;
}

void* receiveMessage(int ds,sockaddr_in* src,int* msgdim) {
	uint16_t net_recvMsgDim;
	int msg_dim;
	int recvBytes;
	unsigned int addrDim;
	void* buffer;

	recvBytes = recvfrom(ds,&net_recvMsgDim,sizeof(uint16_t),0,(struct sockaddr*)src,&addrDim);
	if(recvBytes < sizeof(uint16_t)) {
		perror("Lenght receive error");
		*msgdim = recvBytes;
		return NULL;
    }
    msg_dim = ntohs(net_recvMsgDim);
	buffer = malloc(ntohs(net_recvMsgDim));

	recvBytes = recvfrom(ds,buffer,msg_dim,0,(struct sockaddr*)src,&addrDim);
	if(recvBytes < msg_dim) {
		printf("%d %d\n",recvBytes,msg_dim);
		perror("Message receive error");
		*msgdim = recvBytes;
		return NULL;
	}

	*msgdim = msg_dim;
	return buffer;
}

void sendMessage(int ds,sockaddr_in* dst,char* message,int length) {
	int snddBytes;
	uint16_t nlen = htons(length);
	snddBytes = sendto(ds,&nlen,sizeof(uint16_t),0,(struct sockaddr*)dst,sizeof(*dst));
	if(snddBytes < sizeof(uint16_t)) {
		perror("Length send error");
		return;
	}
	snddBytes = sendto(ds,message,length,0,(struct sockaddr*)dst,sizeof(*dst));
	if(snddBytes < length) {
		perror("Mesage send error");
		return;
	}
}


char* get_ip_from_socket(int socket) {
	sockaddr_in addr;
	unsigned int len = sizeof(sockaddr_in);
	getpeername(socket,(sockaddr*)&addr,&len);
	char *addrstr = malloc(INET_ADDRSTRLEN+1);
	inet_ntop(AF_INET,(void*)&addr.sin_addr,addrstr,len);
	return addrstr;
}

boolean send_int(int ds,sockaddr_in* dst,int d) {
	uint32_t net_int = htonl(d);
	int sended;
	sended = sendto(ds,&net_int,sizeof(uint32_t),0,(struct sockaddr*)dst,sizeof(*dst));
	if(sended < sizeof(uint32_t))
		perror("SendInt error");
	return sended == sizeof(uint32_t);
}

boolean recv_int(int ds,sockaddr_in* src,int* d) {
	uint32_t net_int;
	int recvd; unsigned int addrdim;
	recvd = recvfrom(ds,&net_int,sizeof(uint32_t),0,(struct sockaddr*)src,&addrdim);
	*d = ntohl(net_int);
	if(recvd < sizeof(uint32_t))
		perror("RecvInt error");
	return recvd == sizeof(uint32_t);
}
