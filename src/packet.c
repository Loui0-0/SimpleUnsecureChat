#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "packet.h"

DataPacket* newDataPacket(){
	DataPacket* dataPacket = malloc(sizeof(DataPacket));
	if (dataPacket == NULL){
		return NULL;
	}

	memset(dataPacket, 0, sizeof(DataPacket));
	
	dataPacket->type = P_EMPTY;
	return dataPacket;
}

DataPacket* getDataPacket(void* packedData, packetType typeOfData){
	//data should a defined packet

	DataPacket* dataPacket = newDataPacket();

	if (dataPacket == NULL)
		return NULL;

	dataPacket->type = typeOfData;



    size_t dataSize;

    switch (typeOfData) {
        case P_USERNAME:
            dataSize = sizeof(UsernamePacket);
            break;
        case P_INT:
            dataSize = sizeof(int);
            break;
        case P_MESSAGE:
            dataSize = sizeof(MessagePacket);
            break;
        case P_EMPTY:
            dataSize = 0;
            break;
        default:
            free(dataPacket);
            return NULL;
    }

    if (dataSize > 0) {
        memcpy(dataPacket->rawData, packedData, dataSize);
    }

	return dataPacket;
}

MessagePacket* getFlagMessage(MessageFlag flag){
	MessagePacket* flagMessage = malloc(sizeof(MessagePacket));
		
	if (flagMessage == NULL){
		return NULL;
	}

	flagMessage->flag = flag;
	flagMessage->sentTimestamp = time(NULL);
	strcpy(flagMessage->msgContent, "");
	strcpy(flagMessage->username, "");

	return flagMessage;
}


void DataPacket_deserialize_to_int(DataPacket* dataPacket, int* integer){
	memcpy(integer, dataPacket->rawData, sizeof(int));
}

void DataPacket_deserialize_to_usernamePacket(DataPacket* dataPacket, UsernamePacket* usernamePacket){
	memcpy(usernamePacket, dataPacket->rawData, sizeof(UsernamePacket));
}

void DataPacket_deserialize_to_messagePacket(DataPacket* dataPacket, MessagePacket* messagePacket){
	memcpy(messagePacket, dataPacket->rawData, sizeof(MessagePacket));
}

