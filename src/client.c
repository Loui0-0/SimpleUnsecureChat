
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <signal.h>

#include "client.h"
#include "server.h"
#include "socketUtils.h"
#include "debug.h"
#include "chat.h"
#include "renderer.h"


Renderer* renderer;
volatile sig_atomic_t edit_mode_toggle = 0;


int main(){

	set_signal_action();

	renderer = newRenderer();
	renderer_initialize_console(renderer);

	Chat* displayChat = newChat();

	char username[USERNAMESIZE];
	renderer_print(renderer, "enter username:");
	scanf("%s", username);
	renderer_reset_input(renderer);
	
	int serversPort;
	renderer_print(renderer, "enter server's port nb:");
	scanf("%d", &serversPort);
	renderer_reset_input(renderer);
	
	renderer_print(renderer, "connecting to server's socket...");
	ServerData* serverSocketToSendData = connectToServerSocket("127.0.0.1", serversPort);
	
	renderer_print(renderer, "initializing receiving socket...");
	ServerData* serverSocketToReceiveData = initializeServerSocket();

	if (listen(serverSocketToReceiveData->fd, 1)){
		renderer_print(renderer, "error couldn't listen\n");
	}

	int receivingPort = ntohs(serverSocketToReceiveData->addr.sin_port);
	
	renderer_print(renderer, "sending port nb and username to the server... ");

	//creating username packet
	UsernamePacket usernamePacket;
	strcpy(usernamePacket.username, username);

	DataPacket* portDataPacket = getDataPacket(&receivingPort, P_INT);
	DataPacket* usernamDataPacket = getDataPacket(&usernamePacket, P_USERNAME);

	//sending packet to server
	sendDataPacket(serverSocketToSendData->fd, portDataPacket); 
	sendDataPacket(serverSocketToSendData->fd, usernamDataPacket);
	free(portDataPacket);
	free(usernamDataPacket);


	pthread_t send_thread_id;
	pthread_t receive_thread_id;


	sending_thread_params params_sending;
	params_sending.username = username;
	params_sending.receiver = serverSocketToSendData;
	params_sending.displayChat = displayChat;
	params_sending.renderer = renderer;

	receiving_thread_params params_receiving;
	params_receiving.sender = serverSocketToReceiveData;
	params_receiving.displayChat = displayChat;
	params_receiving.renderer = renderer;

	pthread_create(&send_thread_id,    NULL, sending_thread,   (void*) &params_sending);
	pthread_create(&receive_thread_id, NULL, receiving_thread, (void*) &params_receiving);

    while (1) {
        if (edit_mode_toggle) {
            if (renderer->editMode) {
                renderer_exit_edit_mode(renderer);
            } else {
                renderer_initialize_edit_mode(renderer);
            }
            edit_mode_toggle = 0;
        }
        usleep(100000); 
    }


	pthread_join(send_thread_id, NULL);
	pthread_join(receive_thread_id, NULL);


	close(serverSocketToSendData->fd);
	close(serverSocketToReceiveData->fd);
	pthread_mutex_destroy(&displayChat->mutex);


	return 0;

}

void* receiving_thread(void* params_void) {
    receiving_thread_params* params = (receiving_thread_params*) params_void;
    struct sockaddr_in addr_in;
    socklen_t addr_in_len = sizeof(addr_in);

    int sender_fd = accept(params->sender->fd, (struct sockaddr*) &addr_in, &addr_in_len);
    
    ServerData* server = malloc(sizeof(ServerData));
    server->fd = sender_fd;
    server->addr = addr_in;

    MessagePacket msgReceived;
    DataPacket* dataPacket = newDataPacket();

    while (1) {    
        int out = recvDataPacket(server->fd, dataPacket);
        if (out <= 0) break;

        if (dataPacket->type == P_MESSAGE) {
            DataPacket_deserialize_to_messagePacket(dataPacket, &msgReceived);
            handleMessageLog(params->displayChat, params->renderer, &msgReceived);
        }
    }

    close(server->fd);
    return NULL;
}

void* sending_thread(void* params_void){

    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    renderer_moveCursor(0, w.ws_row);
	sending_thread_params* params = (sending_thread_params*) params_void;
	
	MessagePacket msgToSend;
	MessagePacket msgToSave;
	
	while (1){
		if (!params->renderer->editMode){
			renderer_reset_input(params->renderer);
			//get user input
			fgets(msgToSend.msgContent, MESSAGEMAXLEN, stdin);
		} else {
			fgets(msgToSend.msgContent, MESSAGEMAXLEN, stdin);
			renderer_exit_edit_mode(params->renderer);
		}

		fflush(stdout);

		

		//removes '\n' 
		msgToSend.msgContent[strlen(msgToSend.msgContent)-1] = '\0';
		
		msgToSend.sentTimestamp = time(NULL);


		if (strlen(msgToSend.msgContent) > 0){

			//build packet and send
			DataPacket* dataPacket = getDataPacket(&msgToSend, P_MESSAGE);
			sendDataPacket(params->receiver->fd, dataPacket);
			free(dataPacket);

			//add to local chat
			msgToSave = msgToSend;
			msgToSave.flag = FLAG_MSG_FROM_SELF;
			Chat_append(params->displayChat, time(NULL), &msgToSave);
			printLastEntry(params->renderer, params->displayChat);
		}

	}

	close(params->receiver->fd);
	return NULL;
}

void handleMessageLog(Chat* chat, Renderer* renderer, MessagePacket* msg) {
    char buffer[MESSAGEMAXLEN+100];
    const char* red = "\033[31m";
    const char* green = "\033[32m";
    const char* yellow = "\033[33m";
    const char* reset = "\033[0m";

    switch (msg->flag) {
        case FLAG_NEW_MESSAGE:
            Chat_append(chat, time(NULL), msg);
            printLastEntry(renderer, chat);
            break;

        // Error cases
        case FLAG_COMMAND_ERROR_CHANNEL_UNAVAILABLE:
            snprintf(buffer, sizeof(buffer), "%sError: Channel unavailable%s", red, reset);
            break;
        case FLAG_COMMAND_ERROR_COMMAND_NOT_FOUND:
            snprintf(buffer, sizeof(buffer), "%sError: Unknown command%s", red, reset);
            break;
        case FLAG_CONNECTION_ERROR_SERVER2CLIENT:
            snprintf(buffer, sizeof(buffer), "%sConnection error: Server closed%s", red, reset);
            break;
        case FLAG_COMMAND_ERROR_CHANNEL_CREATION:
            snprintf(buffer, sizeof(buffer), "%sError: Channel creation failed%s", red, reset);
            break;
        case FLAG_SEND_ERROR_NOT_CONNECTED_TO_ANY_CHANNEL:
            snprintf(buffer, sizeof(buffer), "%sError: Not in any channel%s", red, reset);
            break;

        // Success cases
        case FLAG_COMMAND_SUCCESS_CHANNEL_SELECTION:
            snprintf(buffer, sizeof(buffer), "%sSwitched channel: %s%s", green, msg->msgContent, reset);
            break;
        case FLAG_COMMAND_SUCCESS_CHANNEL_CREATION:
            snprintf(buffer, sizeof(buffer), "%sCreated channel: %s%s", green, msg->msgContent, reset);
            break;
        case FLAG_SEND_SUCCESS:
            // snprintf(buffer, sizeof(buffer), "%sMessage delivered%s", green, reset);
            break;

        case FLAG_MSG_FROM_SELF:
            break;

        default:
            snprintf(buffer, sizeof(buffer), "%sUnknown message flag: %d%s", yellow, msg->flag, reset);
            break;
    }
    
    if (msg->flag != FLAG_NEW_MESSAGE && msg->flag != FLAG_MSG_FROM_SELF) {
        renderer_print(renderer, buffer);
    }
}

char* entryToString(Entry* entry) {
    if (!entry) return NULL;
    
    int length = MESSAGEMAXLEN + USERNAMESIZE + 150 + 40;
    
    char* result = malloc(length);
    if (!result) return NULL;

    char sentTimeStr[20];
    strftime(sentTimeStr, sizeof(sentTimeStr), "%H:%M:%S", localtime(&entry->sentTimestamp));

    
    const char* time_color = "\033[36m";      
    const char* self_color = "\033[32m";      
    const char* other_color = "\033[33m";     
    const char* reset = "\033[0m";            


    snprintf(result, length,
        "%s[%s]%s "          
        "%s%s%s: "           
        "%s",                
        time_color, sentTimeStr, reset,
        (entry->flag == FLAG_MSG_FROM_SELF ? self_color : other_color),
        (entry->flag == FLAG_MSG_FROM_SELF ? "you" : entry->username),
        reset,
        entry->msgContent);

    return result;
}


void sigint_handler(int signal){
	if (signal == SIGINT){
		edit_mode_toggle = 1;
	}
}

void printLastEntry(Renderer* renderer, Chat* chat) {
    pthread_mutex_lock(&chat->mutex);
    if (chat->nbEntries > 0) {
    	renderer_print(renderer, entryToString(chat->entries[chat->nbEntries - 1]));
    }
    pthread_mutex_unlock(&chat->mutex);
}


void set_signal_action(void){
	struct sigaction act;
	bzero(&act, sizeof(act));
	act.sa_handler = &sigint_handler;
	sigaction(SIGINT, &act, NULL);
}
