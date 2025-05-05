#ifndef DEBUG_H
#define DEBUG_H

#include "server.h"
#include "chat.h"
#include "socketUtils.h"

void printUser(User* user);
void printEntry(Entry* entry);
void printChat(Chat* chat);
void printChannel(Channel* channel);
void printUserArray(UserArray* userArray);
void printChannelArray(ChannelArray* channelArray);
void printServerData(ServerData* serverData);
void printMessage(MessagePacket* message);
void printClient(DualSocket* dualSocket);

#endif
