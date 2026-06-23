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

ChatCommand parseCommand(char* userInput) {
	if  (strncmp(userInput, "/login", strlen("/login")) == 0){
		return LOGIN_COMMAND;
	} else if (strncmp(userInput, "/history", strlen("/history")) == 0) {
		return MESSAGE_HISTORY;
	} else if (strncmp(userInput, "/join", strlen("/join")) == 0) {
		return JOIN_CHAT;
	} else if (strncmp(userInput, "/create", strlen("/create")) == 0) {
		return CREATE_CHAT;
	} else if (strncmp(userInput, "/details", strlen("/details")) == 0) {
		return SHOW_DETAILS;
	}

	return UNKNOWN;
}

void sendMessageToClient(Client* client, char* message, char* add_info) {
	char msg[128];
	int len = snprintf(msg, sizeof(msg), "%s %s\n", message, add_info);
	printf("[INFO] Sending message: %s", msg);
	send(client->fd, msg, len, 0);

}

int timestamp_curr(char *buf, size_t size) {
	time_t now = time(NULL);
	struct tm tm_now;
	localtime_r(&now, &tm_now);

	return strftime(buf, size, "%Y-%m-%d %H:%M:%S", &tm_now) != 0;
	
}

void sendNewChatMessage(Chatroom* chatroom) { // TO-DO this can be changed to pointers later
	printf("[DEBUG] chatroom ptr = %p\n", (void*)chatroom);   // ← prints BEFORE the deref
	printf("[DEBUG] chatroom->members ptr = %p\n", (void*)chatroom->members);   // ← prints BEFORE the deref
    	printf("[DEBUG] chatroom->members->space = %zu\n", chatroom->members->space);
    	printf("[DEBUG] chatroom->members->size = %zu\n", chatroom->members->size);
	char* keys[16] = {0};
	int members_count = table_get_keys(chatroom->members, keys);
	printf("Debug d");	
	for (int i = 0; i < members_count; i++) {
			printf("Debug");
		Client* client = table_get(clients, keys[i]);
		char senderInfo[256];
		char time_stamp[64];
		timestamp_curr(time_stamp, sizeof(time_stamp));
		for (int j = 0; i < 512; j++) {
			if (!chatroom->messages[j]) continue;
			snprintf(senderInfo, sizeof(senderInfo), "%s %s:",  time_stamp, chatroom->messages[j]->username);
			printf("The client to send message to is %s\n", client->name);
			sendMessageToClient(client, senderInfo, chatroom->messages[j]->content);  
		}		

	}

} 

void sendMessageToChat(Client* client, char* buf) {
	char* username = client->name;
	Chatroom* chatroom = table_get(chatrooms, client->activeChat);
	printf("[DEBUG] Chatroom pointer by activeChat %p\n", chatroom);
	for (int i = 0; i<128; i++) {
		
		if (chatroom->messages[i] == 0) {
			Message* message = malloc(sizeof(struct s_message));
			strncpy(message->username, username, sizeof(message->username));
			strncpy(message->content, buf, sizeof(message->content) - 1);
			chatroom->messages[i] = message;
			break;
		}
	}
	sendNewChatMessage(chatroom);
}


void handleLogin(Client* client, char* buf) {
	//offszotuje login komende
	char* username = buf + strlen("/login ");
	if (client->name[0] == '\0') {
		strncpy(client->name, username, sizeof(client->name) - 1);
		client->name[strcspn(client->name, "\r\n")] = '\0';
		client->isLoggedIn = true;
		sendMessageToClient(client, "User was logged in with username: ", username);
	}
}

void displayCommands(Client* client) {
	char buf[128] = "Type /login to login with your username\n";
	send(client->fd, buf, strlen(buf), 0);
}

Chatroom* createChatroom(Client* client, char* buf) {
	char* name = buf + strlen("/create ");
	Chatroom* chat = malloc(sizeof(struct s_chat_room));
	chat->members = create_table();
	strncpy(chat->name, name, sizeof(chat->name) - 1);
	chat->name[sizeof(chat->name) - 1] = '\0';
	table_set(chatrooms, name, chat);
	Chatroom* createdChat = table_get((ht_table*)chatrooms, name);

	if (!createdChat) {
		printf("[ERROR] Chat not added to chat hash table\n");
		return NULL;
	}
	
	sendMessageToClient(client, "Chatroom created with name: ", createdChat->name);
	char* keys[16] = {0};
	table_get_keys(chatrooms, keys);

	printf("[INFO] Available chatrooms: \n");
	for (int i = 0; i < chatrooms->size; i++) {
		printf("[INFO] Chatname: %s\n", keys[i]);
	}
	return createdChat;
}

void joinChat(Client* client, char* buf) {
	char* chatroom_name = buf + strlen("/join ");

	Chatroom* chatroom = table_get(chatrooms, chatroom_name);
	if (!chatroom) {
		sendMessageToClient(client, "Selected chatroom does not exist for name: ", chatroom_name);
	}

	client->activeChat = chatroom->name;
	table_set(chatroom->members, client->name, client);

	sendMessageToClient(client, "You have joined  the chatroom: ", chatroom->name);
}

Client* createClientSocketAsync(int fd) {
	//client TCP config and connection accept
	struct sockaddr_in client;
	unsigned int len = sizeof(client);

	struct pollfd pfd = { .fd = fd, .events = POLLIN };
	int ret = poll(&pfd, 1, 1000);

	if (ret < 0) { perror("poll"); }
	if (ret == 0) { return NULL; }
	if (!(pfd.revents & POLLIN)) { return NULL; }

	
	//This will only start if there is a connection
	Client* client_s = (Client *)malloc(sizeof(struct s_client)); // mozliwe ze bedzie trzeba wyzerowac
	memset(client_s, 0, sizeof(struct s_client));
	client_s->fd = accept(fd, (struct sockaddr *)&client, &len); 
	memset(client_s->name, 0, sizeof(client_s->name)); 

	client_s->c_sockaddr = client;
	int port = ntohs(client.sin_port);
	client_s->port = port;
	inet_ntop(AF_INET, &client.sin_addr, client_s->ip, sizeof(client_s->ip));
	printf("[INFO] New connection from %s:%d\n", client_s->ip, port);
	displayCommands(client_s);
	return client_s;
}

void handleClient(Client* client) {
	//getting a message
	char buf[512] = {0};
	int conn = recv(client->fd, buf, 511, 0);
	
	//check clients for message
	
	if (conn > 0) {
		if (client->isLoggedIn) {
			switch(parseCommand((char*)&buf[0])) {
				case (MESSAGE_HISTORY): 
					printf("Command was '/history'\n");
					break;
				case (JOIN_CHAT): 
					printf("Command was '/join'\n");
					joinChat(client, buf);
					break;
				case (CREATE_CHAT): 
					printf("Command was '/create'\n");
					createChatroom(client, buf);
					break;
				case (SHOW_DETAILS):
					printf("Command was '/details'\n");
					sendMessageToClient(client, client->name, "");
					break;
				case (UNKNOWN):
					if (client->activeChat) {
						sendMessageToChat(client, buf);
					}
					printf("Command was UNKNOWN\n");
					break;
				case (LOGIN_COMMAND):
					sendMessageToClient(client, "You are already logged in, as a :", client->name);
					break;
			}
		} else {
			switch(parseCommand((char*)&buf[0])) {
				case (LOGIN_COMMAND): 
					printf("Command was '/login'\n");
					handleLogin(client, buf);
					break;
				case (MESSAGE_HISTORY):
				case (JOIN_CHAT): 
				case (CREATE_CHAT):
				case (SHOW_DETAILS):
				case (UNKNOWN):
					printf("User no logged in!\n");
					send(client->fd, "User not logged in\n", strlen("User not logged in"), 0);
			}
		}
	
	} else if (conn == 0 || errno != EINTR) {
		printf("Client disconnected: %s\n", client->ip);
		close(client->fd);
		free(client);
	}

}


