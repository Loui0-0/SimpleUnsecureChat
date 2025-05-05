#ifndef SOCKETUTILS_H
#define SOCKETUTILS_H


#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <pthread.h>

#include "packet.h"

#define NBMAXCLIENT 100



typedef struct{
	int fd;
	struct sockaddr_in addr;
} ServerData;

typedef struct {
	//dual socket system
	ServerData* receiver;
	ServerData* sender;
} DualSocket;


ServerData* initializeServerSocket();
ServerData*  connectToServerSocket(char* ipstr, int port);

int sendDataPacket(int fd, DataPacket* dataPacket);
int recvDataPacket(int fd, DataPacket* dataPacket);

#endif