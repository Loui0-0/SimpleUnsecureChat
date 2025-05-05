#ifndef CHAT_H
#define CHAT_H

#include <time.h>
#include <pthread.h>
#include "socketUtils.h"
#include "packet.h"

// Forward declarations
typedef struct User User;
typedef struct UserArray UserArray;
typedef struct Channel Channel;
typedef struct Chat Chat;
typedef struct Entry Entry;

typedef struct Entry {
    time_t sentTimestamp;
    time_t receivedTimestamp;
    char username[USERNAMESIZE];
    char msgContent[MESSAGEMAXLEN];
    MessageFlag flag;

} Entry;


typedef struct Chat {
    Entry** entries;
    int nbEntries;
    int nbEntriesMax;
    pthread_mutex_t mutex;
} Chat;

typedef struct Channel {
    int channelID;
    int key;
    Chat* chat;
    UserArray* channelUsers;
    pthread_mutex_t mutex;
} Channel;

typedef struct User {
    DualSocket* dualSocket;
    char username[USERNAMESIZE];
    pthread_t threadID;
    Channel* channel;
} User;

typedef struct UserArray {
    User** users;
    int nbUsers;
    int nbUsersMax;
    pthread_mutex_t mutex;
} UserArray;

typedef struct ChannelArray {
    Channel** channelArray;
    int nbChannel;
    int nbChannelMax;
    pthread_mutex_t mutex;
} ChannelArray;



Entry* newEntry_fromMessageReceived(time_t receivedTimestamp, MessagePacket* mesgReceived);
Chat* newChat();
int Chat_append(Chat* chat, time_t receivedTimestamp, MessagePacket* mesgReceived);
Entry* Chat_get(Chat* chat, int index);

Channel* newChannel(int channelID);
ChannelArray* newChannelArray();
int ChannelArray_add(ChannelArray* channelArray, int channelID);
void ChannelArray_removeByID(ChannelArray* channelArray, int channelID);
Channel* ChannelArray_getFromID(ChannelArray* channelArray, int channelID);
void Channel_disconnectUser(User* user);
int Channel_connectUser(Channel* channel, User* user);
int ChannelArray_connectUser(ChannelArray* channelArray, User* user, int channelID);

UserArray* newUserArray();
int UserArray_append(UserArray* users, User* user);
void UserArray_remove(UserArray* users, User* user);
User* UserArray_get(UserArray* users, int index);
User* newUser_fromNewConnection(struct sockaddr_in addr_in, int cfd);

#endif