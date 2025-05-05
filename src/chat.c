#include "chat.h"
#include "socketUtils.h"
#include <time.h>


Entry* newEntry_fromMessageReceived(time_t receivedTimestamp, MessagePacket* msgReceived) {
    Entry* entry = malloc(sizeof(Entry));
    if (!entry) return NULL;
    
    //timestamps
    entry->sentTimestamp = msgReceived->sentTimestamp;
    entry->receivedTimestamp = receivedTimestamp;
    
    //message data
    strncpy(entry->username,    msgReceived->username, USERNAMESIZE);
    strncpy(entry->msgContent,  msgReceived->msgContent, MESSAGEMAXLEN);
    
    //flag
    entry->flag = msgReceived->flag;

    return entry;
}

Chat* newChat() {
    Chat* chat = malloc(sizeof(Chat));
    chat->entries = malloc(sizeof(Entry*) * 10);
    chat->nbEntries = 0;
    chat->nbEntriesMax = 10;
    pthread_mutex_init(&chat->mutex, NULL);
    return chat;
}

int Chat_append(Chat* chat, time_t receivedTimestamp, MessagePacket* mesgReceived) {
    pthread_mutex_lock(&chat->mutex);
    
    Entry* lastEntry = newEntry_fromMessageReceived(receivedTimestamp, mesgReceived);
    if (!lastEntry) return -1;

    if (chat->nbEntries + 1 >= chat->nbEntriesMax) {
        chat->nbEntriesMax *= 2;
        Entry** tmp = realloc(chat->entries, sizeof(Entry*) * chat->nbEntriesMax);
        if (!tmp) return -1;
        chat->entries = tmp;
    }

    chat->entries[chat->nbEntries++] = lastEntry;
    pthread_mutex_unlock(&chat->mutex);

    return 1;
}

Entry* Chat_get(Chat* chat, int index) {
    if (index < chat->nbEntries) return chat->entries[index];
    return NULL;
}

Channel* newChannel(int channelID) {
    Channel* channel = malloc(sizeof(Channel));
    if (!channel) return NULL;

    channel->chat = newChat();
    if (!channel->chat) {
        free(channel);
        return NULL;
    }

    channel->channelUsers = newUserArray();
    if (!channel->channelUsers) {
        free(channel->chat);
        free(channel);
        return NULL;
    }

    channel->channelID = channelID;
    pthread_mutex_init(&channel->mutex, NULL);

    return channel;
}

ChannelArray* newChannelArray() {
    ChannelArray* channelArray = malloc(sizeof(ChannelArray));
    if (!channelArray) return NULL;

    channelArray->nbChannel = 0;
    channelArray->nbChannelMax = 10;
    channelArray->channelArray = malloc(sizeof(Channel*) * channelArray->nbChannelMax);
    
    if (!channelArray->channelArray) {
        free(channelArray);
        return NULL;
    }
    
    pthread_mutex_init(&channelArray->mutex, NULL);
    return channelArray;
}

int ChannelArray_add(ChannelArray* channelArray, int channelID) {
    Channel* channel = newChannel(channelID);
    if (!channel) return -1;

    if (channelArray->nbChannel + 1 >= channelArray->nbChannelMax) {
        channelArray->nbChannelMax *= 2;
        Channel** tmp = realloc(channelArray->channelArray, sizeof(Channel*) * channelArray->nbChannelMax);
        if (!tmp) {
            free(channel);
            return -1;
        }
        channelArray->channelArray = tmp;
    }

    channelArray->channelArray[channelArray->nbChannel++] = channel;
    return 1;
}

void ChannelArray_removeByID(ChannelArray* channelArray, int channelID) {
    int indexToRemove = -1;
    for (int i = 0; i < channelArray->nbChannel; ++i) {
        if (channelArray->channelArray[i]->channelID == channelID) {
            indexToRemove = i;
            break;
        }
    }

    if (indexToRemove >= 0) {
        for (int j = indexToRemove; j < channelArray->nbChannel - 1; ++j) {
            channelArray->channelArray[j] = channelArray->channelArray[j+1];
        }
        channelArray->nbChannel--;
    }
}

Channel* ChannelArray_getFromID(ChannelArray* channelArray, int channelID) {

    for (int i = 0; i < channelArray->nbChannel; ++i) {
        if (channelArray->channelArray[i]->channelID == channelID) {
            return channelArray->channelArray[i];
        }
    }


    return NULL;
}

void Channel_disconnectUser(User* user) {
    if (user->channel != NULL) {
        UserArray_remove(user->channel->channelUsers, user);
        user->channel = NULL;
    }
}

int Channel_connectUser(Channel* channel, User* user) {
    
    if (user->channel) return -1;
    if (UserArray_append(channel->channelUsers, user) != 1) return -1;

    user->channel = channel;
    return 1;
}

int ChannelArray_connectUser(ChannelArray* channelArray, User* user, int channelID) {
    Channel* channelRequested = ChannelArray_getFromID(channelArray, channelID);
    if (!channelRequested) return -1;


    Channel_disconnectUser(user);
    return Channel_connectUser(channelRequested, user);
}

UserArray* newUserArray() {
    UserArray* userArray = malloc(sizeof(UserArray));
    if (!userArray) return NULL;

    userArray->nbUsers = 0;
    userArray->nbUsersMax = 10;
    userArray->users = malloc(sizeof(User*) * userArray->nbUsersMax);
    
    if (!userArray->users) {
        free(userArray);
        return NULL;
    }
    return userArray;
}

int UserArray_append(UserArray* userArray, User* user) {
    
    if (userArray->nbUsers + 1 >= userArray->nbUsersMax) {
        userArray->nbUsersMax *= 2;
        User** tmp = realloc(userArray->users, sizeof(User*) * userArray->nbUsersMax);
        if (!tmp) return -1;
        userArray->users = tmp;
    }



    userArray->users[userArray->nbUsers++] = user;
    return 1;
}

void UserArray_remove(UserArray* userArray, User* user) {
    int indexToRemove = -1;
    for (int i = 0; i < userArray->nbUsers; ++i) {
        if (userArray->users[i] == user) {
            indexToRemove = i;
            break;
        }
    }

    if (indexToRemove >= 0) {
        for (int j = indexToRemove; j < userArray->nbUsers - 1; ++j) {
            userArray->users[j] = userArray->users[j+1];
        }
        userArray->nbUsers--;
    }
}

User* UserArray_get(UserArray* userArray, int index) {
    if (index < userArray->nbUsers) return userArray->users[index];
    return NULL;
}

User* newUser_fromNewConnection(struct sockaddr_in addr_in, int cfd) {
    User* user = malloc(sizeof(User));
    if (!user) return NULL;

    DualSocket* dualSocket = malloc(sizeof(DualSocket));
    if (!dualSocket) {
        free(user);
        return NULL;
    }

    ServerData* sender = malloc(sizeof(ServerData));
    if (!sender) {
        free(dualSocket);
        free(user);
        return NULL;
    }

    user->dualSocket = dualSocket;
    user->dualSocket->sender = sender;
    user->dualSocket->sender->fd = cfd;
    user->dualSocket->sender->addr = addr_in;
    user->channel = NULL;
    return user;
}