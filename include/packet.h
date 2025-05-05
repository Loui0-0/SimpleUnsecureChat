#ifndef PACKET_H
#define PACKET_H
#include <time.h>

//10ko
#define DATAPACKETSIZE 10000
#define MESSAGEMAXLEN 5000
#define USERNAMESIZE 100

typedef struct DataPacket DataPacket;
typedef struct UsernamePacket UsernamePacket;
typedef struct PortPacket PortPacket;
typedef struct MessagePacket MessagePacket;

typedef enum packetType{
	P_EMPTY = 0,
	P_USERNAME = 1,	
	P_INT = 2,
	P_MESSAGE  = 3

}packetType;

typedef enum {
    // Error flags
    FLAG_COMMAND_ERROR_CHANNEL_UNAVAILABLE = -1,
    FLAG_COMMAND_ERROR_COMMAND_NOT_FOUND   = -2,
    FLAG_CONNECTION_ERROR_SERVER2CLIENT    = -3,
    FLAG_COMMAND_ERROR_CHANNEL_CREATION    = -4,
    FLAG_SEND_ERROR_NOT_CONNECTED_TO_ANY_CHANNEL = -5,

    // Success flags
    FLAG_COMMAND_SUCCESS_CHANNEL_SELECTION = 1,
    FLAG_COMMAND_SUCCESS_CHANNEL_CREATION  = 4,
    FLAG_SEND_SUCCESS = 6,

    // Other flags
    FLAG_NEW_MESSAGE = 5,
    FLAG_MSG_FROM_SELF = 7
} MessageFlag;



typedef struct DataPacket{
	packetType type;
	char rawData[DATAPACKETSIZE];
} DataPacket;

typedef struct UsernamePacket{
	char username[USERNAMESIZE];	
} UsernamePacket;

typedef struct MessagePacket{
	MessageFlag flag;
	char username[USERNAMESIZE];
	time_t sentTimestamp;
	char msgContent[MESSAGEMAXLEN];
} MessagePacket;

DataPacket* newDataPacket();
DataPacket* getDataPacket(void* dataToPack, packetType typeOfData);

MessagePacket* getFlagMessage(MessageFlag flag);


void DataPacket_deserialize_to_int(DataPacket* dataPacket, int* integer);
void DataPacket_deserialize_to_usernamePacket(DataPacket* dataPacket, UsernamePacket* usernamePacket);
void DataPacket_deserialize_to_messagePacket(DataPacket* dataPacket, MessagePacket* messagePacket);


#endif