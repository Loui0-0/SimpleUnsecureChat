
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <pthread.h>


#include "server.h"

#include "socketUtils.h"
#include "debug.h"

#include "chat.h"

#include "packet.h"



int main(){
	
	UserArray* connectedUsers = newUserArray();
	ChannelArray* channels = newChannelArray();
	ServerData* srvdat = initializeServerSocket();
	
	//create the default Channel
	ChannelArray_add(channels, 0);
	
	//read port
	printf("server open on port %d: \n", ntohs(srvdat->addr.sin_port));
	


	if (listen(srvdat->fd, NBMAXCLIENT)){
		printf("error listen\n");
	}

	struct sockaddr_in addr_in;
	socklen_t addr_in_len = sizeof(addr_in);
	
	while (1){
		int cfd = accept (srvdat->fd, (struct sockaddr*) &addr_in, &addr_in_len);
		
		//create a new user
		User* user = newUser_fromNewConnection(addr_in, cfd);

		//add it to the list of users
		UserArray_append(connectedUsers, user);

		//handle the user in a separate thread
		user_thread_params* params = get_user_thread_params(user, channels);
		if (pthread_create(&user->threadID, NULL, user_thread, (void *)params) == 0) {
			printf("Created new client thread successfully\n");
		} else {
			perror("Failed to create thread");
		}
		
	}


	close(srvdat->fd); 
	return 0;

}



user_thread_params* get_user_thread_params(User* user, ChannelArray* channels){
	user_thread_params* params = malloc(sizeof(user_thread_params));
	if (!params) {
		perror("malloc failed for params");
		exit(EXIT_FAILURE);
	}
	params->user = user;
	params->channels = channels;

	return params;
}




void* user_thread(void* params_void){

	user_thread_params* params = (user_thread_params*)params_void;
	
	//get user 
	ChannelArray* channels = params->channels;
	User* user = params->user;
	
	
	// retreive user informations
	int clientReceivingPort;

	//data container
	DataPacket* dataPacket = newDataPacket();

	//port
	recvDataPacket(user->dualSocket->sender->fd, dataPacket);
	DataPacket_deserialize_to_int(dataPacket, &clientReceivingPort);

	//username
	UsernamePacket usernamePacket;
	recvDataPacket(user->dualSocket->sender->fd, dataPacket);
	DataPacket_deserialize_to_usernamePacket(dataPacket, &usernamePacket);
	strcpy(user->username, usernamePacket.username);


	//connection to client's receiver socket
	ServerData* receiver = connectToServerSocket(inet_ntoa(user->dualSocket->sender->addr.sin_addr), clientReceivingPort);
	user->dualSocket->receiver = receiver;

	//connect to default channel
	ChannelArray_connectUser(channels, user, 0);

	MessagePacket msgReceived;
	
	strcpy(msgReceived.username, user->username);
	while (1){
		int out = recvDataPacket(user->dualSocket->sender->fd, dataPacket);
		
		if (out <= 0){
			break;
		}

		if (dataPacket->type == P_MESSAGE){
			DataPacket_deserialize_to_messagePacket(dataPacket, &msgReceived);
			handleMessage(channels, user, &msgReceived);
		}
	}

	close(user->dualSocket->sender->fd);
	close(user->dualSocket->receiver->fd);
	return (void *) 1;
}


void handleMessage(ChannelArray* channels, User* user, MessagePacket* msgReceived){
	
	// handle commands 
	if (msgReceived->msgContent[0] == '/'){
		handleCommand(channels, user, msgReceived);
		
	} else if (user->channel != NULL){

		//saving the message
		Chat_append(user->channel->chat, time(NULL), msgReceived);

		//message packing
		MessagePacket incomingMessage;
		//	flag
		incomingMessage.flag          = FLAG_NEW_MESSAGE;
		incomingMessage.sentTimestamp = msgReceived->sentTimestamp;
		//	username
		strcpy(incomingMessage.username  , user->username);
		//	content
		strcpy(incomingMessage.msgContent, msgReceived->msgContent);

		DataPacket* dataPacket = getDataPacket(&incomingMessage, P_MESSAGE);

		//send to every channel user the message
		for (int i = 0; i < user->channel->channelUsers->nbUsers; i++){
			//get user
			User* userOfChannel = UserArray_get(user->channel->channelUsers, i);
			
			if (user == userOfChannel) continue;
			sendDataPacket(userOfChannel->dualSocket->receiver->fd, dataPacket);
		}

		sendFlagMessageToUser(user, FLAG_SEND_SUCCESS);
	} else {
		sendFlagMessageToUser(user, FLAG_SEND_ERROR_NOT_CONNECTED_TO_ANY_CHANNEL);
	}
}



void handleCommand(ChannelArray* channels, User* user, MessagePacket* msgReceived){
	
		
	//get command name and params
	char commandName[100];
	char commandParam1[100];
	sscanf(msgReceived->msgContent,"/%s %s", commandName, commandParam1);

	//handle the found command

	//channel selection
	if (strcmp(commandName, "selectChannel") == 0 || strcmp(commandName, "sc") == 0){
		
		//get requested channel ID 
		int channelID = atoi(commandParam1);

		//attempt connection
		if (ChannelArray_connectUser(channels, user, channelID) == -1)
			sendFlagMessageToUser(user, FLAG_COMMAND_ERROR_CHANNEL_UNAVAILABLE);
		else
			sendFlagMessageToUser(user, FLAG_COMMAND_SUCCESS_CHANNEL_SELECTION);
	
	//channel creation 
	} else if(strcmp(commandName, "createChannel") == 0){ 
		
		//get requested channel ID 
		int channelID = atoi(commandParam1);
		
		//attempt channel creation
		if (ChannelArray_add(channels, channelID) == -1)
			sendFlagMessageToUser(user, FLAG_COMMAND_ERROR_CHANNEL_CREATION);
		else
			sendFlagMessageToUser(user, FLAG_COMMAND_SUCCESS_CHANNEL_CREATION);

		//attempt connection
		if (ChannelArray_connectUser(channels, user, channelID) == -1)
			sendFlagMessageToUser(user, FLAG_COMMAND_ERROR_CHANNEL_UNAVAILABLE);
		else
			sendFlagMessageToUser(user, FLAG_COMMAND_SUCCESS_CHANNEL_SELECTION);

	} else {
		sendFlagMessageToUser(user,FLAG_COMMAND_ERROR_COMMAND_NOT_FOUND);
	}
}


int sendFlagMessageToUser(User* user, int flag){
	MessagePacket* flagMessage = getFlagMessage(flag);
	if (flagMessage == NULL){
		return -1;
	}

	DataPacket* dataPacket = getDataPacket(flagMessage, P_MESSAGE);
	return sendDataPacket(user->dualSocket->receiver->fd, dataPacket);
	free(dataPacket);
}


void server_disconnectUser(UserArray* users, ChannelArray* channels, User* user){

}
