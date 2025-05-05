#include "debug.h"

#include "server.h"
#include "chat.h"
#include "socketUtils.h"
#include "packet.h"


// Print User details
void printUser(User* user) {
    if (!user) {
        printf("User: NULL\n");
        return;
    }
    printf("User: %s (Thread ID: %lu)\n", user->username, user->threadID);
    if (user->dualSocket) {
        printClient(user->dualSocket);
    }
}

// Print an Entry
void printEntry(Entry* entry) {
    if (!entry) {
        printf("Entry: NULL\n");
        return;
    }
    printf("Entry: \n");
    printf("  Sent: %s", ctime(&entry->sentTimestamp));
    printf("  Received: %s", ctime(&entry->receivedTimestamp));
    printf("  username: %s\n", entry->username);
    printf("  flag: %d\n", entry->flag);

}

// Print Chat details
void printChat(Chat* chat) {
    if (!chat) {
        printf("Chat: NULL\n");
        return;
    }
    printf("Chat: %d/%d Entries\n", chat->nbEntries, chat->nbEntriesMax);
    for (int i = 0; i < chat->nbEntries; i++) {
        printEntry(chat->entries[i]);
    }
}

// Print Channel details
void printChannel(Channel* channel) {
    if (!channel) {
        printf("Channel: NULL\n");
        return;
    }
    printf("Channel ID: %d, Key: %d\n", channel->channelID, channel->key);
    if (channel->chat) {
        printChat(channel->chat);
    }
    if (channel->channelUsers) {
        printUserArray(channel->channelUsers);
    }
}

// Print UserArray details
void printUserArray(UserArray* userArray) {
    if (!userArray) {
        printf("UserArray: NULL\n");
        return;
    }
    printf("UserArray: %d/%d Users\n", userArray->nbUsers, userArray->nbUsersMax);
    for (int i = 0; i < userArray->nbUsers; i++) {
        printUser(userArray->users[i]);
    }
}

// Print ChannelArray details
void printChannelArray(ChannelArray* channelArray) {
    if (!channelArray) {
        printf("ChannelArray: NULL\n");
        return;
    }
    printf("ChannelArray: %d/%d Channels\n", channelArray->nbChannel, channelArray->nbChannelMax);
    for (int i = 0; i < channelArray->nbChannel; i++) {
        printChannel(channelArray->channelArray[i]);
    }
}

// Print ServerData details
void printServerData(ServerData* serverData) {
    if (!serverData) {
        printf("ServerData: NULL\n");
        return;
    }
    printf("ServerData - FD: %d, IP: %s, Port: %d\n",
           serverData->fd,
           inet_ntoa(serverData->addr.sin_addr),
           ntohs(serverData->addr.sin_port));
}

// Print Message details
void printMessage(MessagePacket* message) {
    if (!message) {
        printf("Message: NULL\n");
        return;
    }
    printf("Message - Flag: %d\n", message->flag);
    printf("  username: %s\n", message->username);
    printf("  Sent: %s", ctime(&message->sentTimestamp));
    printf("  Content: %s\n", message->msgContent);
}

// Print Client details
void printClient(DualSocket* dualSocket) {
    if (!dualSocket) {
        printf("DualSocket: NULL\n");
        return;
    }
    printf("DualSocket:\n");
    printf("  Sender: ");
    printServerData(dualSocket->sender);
    printf("  Receiver: ");
    printServerData(dualSocket->receiver);
}