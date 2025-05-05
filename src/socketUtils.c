#include "socketUtils.h"
#include "packet.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <pthread.h>


ServerData* initializeServerSocket(){
	
	//create a new socket
	const int fd = socket( AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in addr;

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = 0;

	if (bind(fd, (struct sockaddr *) &addr,  sizeof(addr)) != 0){
		printf("error when binding\n");
		exit(0);
	}

	socklen_t addr_len = sizeof(addr);
	getsockname(fd,(struct sockaddr *) &addr, &addr_len);
	
	ServerData* srvdat = malloc(sizeof(ServerData));
	srvdat->fd = fd;
	srvdat->addr = addr;

	return srvdat;
}

ServerData* connectToServerSocket(char* ipstr, int port){

	//create a new socket
	const int fd = socket( AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;

	addr.sin_port = htons((short) port);

	char addrstr[100];
	snprintf(addrstr, sizeof(addrstr), "%s", ipstr);


	inet_pton(AF_INET, addrstr, &addr.sin_addr);

	if (connect(fd, (struct sockaddr*) &addr, sizeof(addr))){
		printf("error connect");
		exit(0);
	}
	
	ServerData* srvdat = malloc(sizeof(ServerData));
	srvdat->fd = fd;
	srvdat->addr = addr;

	return srvdat;
}


int sendDataPacket(int fd, DataPacket* dataPacket){
	return send(fd, dataPacket, sizeof(DataPacket), 0);
}
int recvDataPacket(int fd, DataPacket* dataPacket){
	return recv(fd, dataPacket, sizeof(DataPacket), 0);
}