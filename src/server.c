

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <netdb.h>
#include <poll.h>
#include <errno.h>

#define HOST "127.0.0.1"
#define DEFAULT_PORT "9090"
#define BACKLOG 10 // how many pending connections queue holds

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
	//add structure that holds details about chat users
};

typedef struct s_chat_room Chatroom;

struct s_client {
	int fd;
	int port;
	char ip[INET_ADDRSTRLEN];
	char name[64];
	bool isLoggedIn;
	Chatroom* activeChat;
	
	struct s_message messages[128];
	struct sockaddr_in c_sockaddr;
};

typedef struct s_client Client;

bool isRunning= true;

ChatCommand parseCommand(char* userInput) {
	printf("Raw command from user: %s\n", userInput);
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

int init_server(unsigned int port) {

	struct sockaddr_in sock_addr;
	memset(&sock_addr, 0, sizeof(struct sockaddr_in));

	int socket_file_descriptor;
	socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);

	if (socket_file_descriptor < 0) {
	 	printf("Error while creating a socket\n");
		return -1;
	}

	printf("Socket was created: %i\n", socket_file_descriptor);

	// preapre sock addr data for binding
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons(port);

	int opt = 1;
	if (setsockopt(socket_file_descriptor, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
		perror("setsockopt");
		return -1;
	}

	// converts ipv4 from text to binary
	inet_pton(AF_INET, HOST, &sock_addr.sin_addr);
	if (bind(socket_file_descriptor,(struct sockaddr *) &sock_addr, sizeof(sock_addr)) < 0) {
		printf("Socket binding failed \n");
		return -1;
	}
	
	listen(socket_file_descriptor, 20);
	
	return socket_file_descriptor;
}

void addClientToList(Client* client, Client* client_list[]) {

	for (int i = 0; i<10; i++) {
		if (client_list[i] == 0) {
			client_list[i] = client;
			break;
		}
    	}
}

void handleLogin(Client* client, char* buf) {
	//offszotuje login komende
	char* username = buf + strlen("/login ");
	printf("%s\n", client->name);
	if (client->name[0] == '\0') {
		strncpy(client->name, username, sizeof(client->name) - 1);
		client->name[strcspn(client->name, "\r\n")] = '\0';
		client->isLoggedIn = true;
		char msg[128];
		int len = snprintf(msg, sizeof(msg), "User was logged in with username: %s\n", username);
		send(client->fd, msg, len, 0);
		printf("%s\n",msg);
	}
}

void displayCommands(Client* client) {
	char buf[128] = "Type /login to login with your username\n";
	send(client->fd, buf, strlen(buf), 0);
}

Chatroom* createChatroom(Chatroom* chatrooms[], char* buf) {
	char* name = buf + strlen("/login ");
	//find empty space for chatroom
	for(int i = 0; i<10; i++) {
		if (chatrooms[i]->name[0] == '\0') {
			chatrooms[i] = malloc(sizeof(struct s_chat_room));
			memset(chatrooms[i], 0, sizeof(struct s_chat_room));
			strncpy(chatrooms[i]->name, name, sizeof(chatrooms[i]->name) - 1);
			printf("Chatroom created with name: %s\n", name);
			return chatrooms[i];
			break;
		}
	}

	printf("No space for new chatroom, wait");
	return NULL;

}

void joinChat(Chatroom* chatrooms[], Client* client, char* buf) {
	char* chatroom_name = buf + strlen("/join ");
	printf("Chatroom to join: %s\n", chatroom_name);
	for (int i = 0; i<10; i++) {
		
		if (strncmp(chatrooms[i]->name, chatroom_name, strlen(chatrooms[i]->name)) == 0 ) {
			client->activeChat = chatrooms[i];
			break;
		}

	}

}

void sendMessageToChat(Client* client, char* buf) {
	char* username = client->name;
	Chatroom* chatroom = client->activeChat;

	for (int i = 0; i<128; i++) {
		
		if (chatroom->messages[i] == NULL) {
			Message* message = malloc(sizeof(struct s_message));
			strncpy(message->username, username, sizeof(message->username) - 1);
			strncpy(message->content, buf, sizeof(message->content) - 1);
			chatroom->messages[i] = message;
		}

	}
	
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
	printf("New connection from %s:%d\n", client_s->ip, port);
	displayCommands(client_s);
	return client_s;
}

void sendMessageToClient(Client* client, char* message) {
	char msg[128];
	int len = snprintf(msg, sizeof(msg), "%s\n", message);
	send(client->fd, msg, len, 0);

}


void handleClient(Chatroom* chatrooms[], Client* client_list[], int i) {
	//getting a message
	char buf[512] = {0};
	int conn = recv(client_list[i]->fd, buf, 511, 0);
	
	//check clients for message
	
	if (conn > 0) {
		if (client_list[i]->isLoggedIn) {
			switch(parseCommand((char*)&buf[0])) {
				case (MESSAGE_HISTORY): 
					printf("Command was '/history'\n");
					break;
				case (JOIN_CHAT): 
					printf("Command was '/join'\n");
					joinChat(chatrooms, client_list[i], buf);
					break;
				case (CREATE_CHAT): 
					printf("Command was '/create'\n");
					createChatroom(chatrooms, buf);
					break;
				case (SHOW_DETAILS):
					printf("Command was '/details'\n");
					sendMessageToClient(client_list[i], client_list[i]->name);
					break;
				case (UNKNOWN):
					if (client_list[i]->activeChat) {
						sendMessageToChat(client_list[i], buf);
					}
					printf("Command was UNKNOWN\n");
					break;
			}
		} else {
			switch(parseCommand((char*)&buf[0])) {
				case (LOGIN_COMMAND): 
					printf("Command was '/login'\n");
					handleLogin(client_list[i], buf);
					break;
				case (MESSAGE_HISTORY):
				case (JOIN_CHAT): 
				case (CREATE_CHAT):
				case (SHOW_DETAILS):
				case (UNKNOWN):
					printf("User no logged in!\n");
					send(client_list[i]->fd, "User not logged in\n", strlen("User not logged in"), 0);
			}
		}
	
	} else if (conn == 0 || errno != EINTR) {
		printf("Client disconnected: %s\n", client_list[i]->ip);
		close(client_list[i]->fd);
		free(client_list[i]);
		client_list[i] = NULL;
	}

}


void main_loop(int fd, Client* client_list[], Chatroom* chatrooms[]) {
	
	struct pollfd fds[11]; // as I will server 10 clients + my server socket
	memset(fds, 0, sizeof(fds));
	
	fds[0].fd = fd;
	fds[0].events = POLLIN;
	int nfds = 1;

	// Maping clients to my array
	int client_index[10];
	for (int i = 0; i< 10; i++) {
		if (client_list[i]) {
			client_index[nfds - 1] = i;
			fds[nfds].fd = client_list[i]->fd;
			fds[nfds].events = POLLIN;
			nfds++;
		}
	}

	int ret = poll(fds, nfds, -1); // -1 allows to block until something happens
	if (ret < 0) { perror("poll"); return; }
	
	if (fds[0].revents & POLLIN) {
		Client* client = createClientSocketAsync(fd);
		if (client != NULL)
			addClientToList(client, client_list);
	}

	for (int j = 1; j < nfds; j++) {
		if (fds[j].revents & POLLIN) {
			handleClient(chatrooms, client_list, client_index[j - 1]);
		}
	}
}

/* TO-DO: Add htons or other combination to convert from network byte order to host or vice versa */
int main(int argc, char *argv[])
{
	Client* client_list[10] = {0};
	Chatroom* chatrooms[12] = {0};
	unsigned int port;
	int socket_file_descriptor;
	
	memset(client_list, 0, sizeof(client_list));
	memset(chatrooms, 0, sizeof(chatrooms));

	if (argc < 2) {
	 printf("Port NOT provided, defaulting to %s \n", DEFAULT_PORT);
	 port = atoi((char *) DEFAULT_PORT);
	} else {
	port = atoi(argv[1]);
	}

	printf("Port that was provided %d \n", port);
	
	socket_file_descriptor = init_server(port);

	while(isRunning) {
		main_loop(socket_file_descriptor, client_list, chatrooms);
	}

	close(socket_file_descriptor);
	printf("It works\n");
	return 0;
}
