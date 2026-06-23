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
#include "client.h"

#ifndef SUPER_HEADER
#include "c_datastructures.h"
#endif

#define HOST "127.0.0.1"
#define DEFAULT_PORT "9090"
#define BACKLOG 10 // how many pending connections queue holds

bool isRunning= true;

HashTable* chatrooms = NULL;
HashTable* clients = NULL;

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


void main_loop(int fd, Client* client_list[], HashTable* chatrooms) {
	
	struct pollfd fds[11]; // as I will server 10 clients + my server socket
	memset(fds, 0, sizeof(fds));
	
	fds[0].fd = fd;
	fds[0].events = POLLIN;
	int nfds = 1;

	// Maping clients to my array
	int client_index[16];
	for (int i = 0; i< 16; i++) {
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
			for(int i = 0; i<16; i++) {
				if (client_list[i] == 0) client_list[i] = client;
				break;
			}
	}

	for (int j = 1; j < nfds; j++) {
		if (fds[j].revents & POLLIN) {
			handleClient(client_list[client_index[j - 1]]);
		}
	}
}

/* TO-DO: Add htons or other combination to convert from network byte order to host or vice versa */
int main(int argc, char *argv[])
{	chatrooms = create_table();
	clients = create_table();

	Client* client_list[16] = {0};
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
		main_loop(socket_file_descriptor, client_list, chatrooms);
	}

	close(socket_file_descriptor);
	table_destroy(chatrooms);
	return 0;
}
