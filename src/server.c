

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

#include <poll.h>

#define HOST "127.0.0.1"
#define DEFAULT_PORT "9090"
#define BACKLOG 10 // how many pending connections queue holds

struct s_client {
	int fd;
	int port;
	char ip[INET_ADDRSTRLEN];
	char name[64];
	struct sockaddr_in c_sockaddr;
};

typedef struct s_client Client;


bool isRunning= true;

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

void requestUsername(Client* client) {

	char buf[128] = "Podaj nazwe użytkownika\n";
	send(client->fd, buf, 128, 0);
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

	client_s->c_sockaddr = client;
	int port = ntohs(client.sin_port);
	client_s->port = port;
	inet_ntop(AF_INET, &client.sin_addr, client_s->ip, sizeof(client_s->ip));
	printf("New connection from %s:%d\n", client_s->ip, port);
	requestUsername(client_s);
	return client_s;
}


void recvAndSendBackMessage(Client* client_list[]) {
	//getting a message
	char buf[512] = {0};

	//check clients for messages

	for (int i=0; i<10; i++) {

		memset(buf, 0, sizeof buf);
		if (!client_list[i]) continue; // skips enty slots, keeps on going

		/*I need to verify if client send something but with using poll()*/

		struct pollfd client_poll_fd = {
		.fd = client_list[i]->fd,
		.events = POLLIN
		};

		int ret = poll(&client_poll_fd, 1, 1000);

		if (ret < 0) {
			perror("poll");
		}
		

		if (client_poll_fd.revents & POLLIN) {
			int conn = recv(client_list[i]->fd, buf, 511, 0);
			if (client_list[i]->name[0] == '\0') {
				strcpy(client_list[i]->name, buf);
				printf("Setting client name to %s\n", client_list[i]->name);
			}
			if (conn > 0) { 
				printf("\nMessage from client: \n%s\n", buf);
				printf("Client with descriptor id %d \n", client_list[i]->fd);
				send(client_list[i]->fd, buf, conn, 0);
			} else if (errno == EINTR || conn == 0) {
				printf("Connection closed\n");
				close(client_list[i]->fd);
				free(client_list[i]);
				client_list[i] = NULL;
			}
		}
	}

}


void main_loop(int fd, Client* client_list[]) { 
	Client* client = createClientSocketAsync(fd);
	if (client != NULL)
		addClientToList(client, client_list);
	recvAndSendBackMessage(client_list);	
}
/* TO-DO: Add htons or other combination to convert from network byte order to host or vice versa */
int main(int argc, char *argv[])
{
	Client* client_list[10] = {0};
	memset(client_list, 0, sizeof(client_list));
	unsigned int port;
	
	int socket_file_descriptor;
	if (argc < 2) {
	 printf("Port NOT provided, defaulting to %s \n", DEFAULT_PORT);
	 port = atoi((char *) DEFAULT_PORT);
	} else {
	port = atoi(argv[1]);
	}

	printf("Port that was provided %d \n", port);
	
	socket_file_descriptor = init_server(port);

	while(isRunning) {
		main_loop(socket_file_descriptor, client_list);
	}

	close(socket_file_descriptor);
	printf("It works\n");
	return 0;
}
