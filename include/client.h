#ifndef CLIENT_H
#define CLIENT_H

#include "socketUtils.h"
#include <time.h>
#include "chat.h"
#include "renderer.h"


typedef struct {
	ServerData* sender;
	Chat* displayChat;
	Renderer* renderer;
} receiving_thread_params;

typedef struct {
	char* username;
	ServerData* receiver;
	Chat* displayChat;
	Renderer* renderer;
} sending_thread_params;


void* receiving_thread(void* params_void);
void* sending_thread(void* params_void);
char* entry_to_string(const Entry* entry);
void set_signal_action(void);
char* entryToString(Entry* entry);
void printLastEntry(Renderer* renderer, Chat* chat);
void handleMessageLog(Chat* chat, Renderer* renderer, MessagePacket* msg);
#endif