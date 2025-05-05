#ifndef SERVER_H
#define SERVER_H
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

#include "chat.h"
#include "packet.h"



void *user_thread(void* params_void);


typedef struct {
	User* user;
	ChannelArray* channels;
} user_thread_params;

user_thread_params* get_user_thread_params(User* user, ChannelArray* channels);

void handleMessage(ChannelArray* channels, User* author, MessagePacket* msgReceived);
void handleCommand(ChannelArray* channels, User* author, MessagePacket* msgReceived);
int sendFlagMessageToUser(User* user, int flag);


#endif