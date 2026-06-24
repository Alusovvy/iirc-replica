#define _GNU_SOURCE
#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <netdb.h>
#include <poll.h>
#include <errno.h>

#include "c_datastructures.h"

#define HOST "127.0.0.1"
#define DEFAULT_PORT "9090"
#define BACKLOG 10 // how many pending connections queue holds

extern HashTable* chatrooms;
extern HashTable* clients;

typedef enum {
	LOGIN_COMMAND,
	MESSAGE_HISTORY,
	JOIN_CHAT,
	CREATE_CHAT,
	SHOW_DETAILS,
	UNKNOWN
} ChatCommand;


struct s_message {
	
	char username[64];
	char content[512];

}; // TO-DO: Expand this later on to Hash-Table

typedef struct s_message Message;

struct s_chat_room {
	char name[64];
	struct s_message* messages[512];
	HashTable* members;
};

typedef struct s_chat_room Chatroom;

struct s_client {
	int fd;
	int port;
	char ip[INET_ADDRSTRLEN];
	char name[64];
	bool isLoggedIn;
	char* activeChat;
	
	struct s_message messages[128];
	struct sockaddr_in c_sockaddr;
};

typedef struct s_client Client;

ChatCommand parseCommand(char*); 
void sendMessageToClient(Client*, char*, char*);
int timestamp_curr(char*, size_t);
void sendNewChatMessage(Chatroom*);
void sendMessageToChat(Client*, char*);
void handleLogin(Client*, char*);
void displayCommands(Client*);
Chatroom* createChatroom(Client*, char*);
void joinChat(Client*, char*);
Client* createClientSocketAsync(int);
void handleClient(Client*);
