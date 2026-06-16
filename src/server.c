

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <netdb.h>

#define HOST "127.0.0.1"
#define DEFAULT_PORT "9090"


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

	// converts ipv4 from text to binary
	inet_pton(AF_INET, HOST, &sock_addr.sin_addr);
	if (bind(socket_file_descriptor,(struct sockaddr *) &sock_addr, sizeof(sock_addr)) < 0) {
		printf("Socket binding failed \n");
		return -1;
	}
	
	listen(socket_file_descriptor, 20);
	
	return socket_file_descriptor;
}

void main_loop(int fd) { 
	
	//client TCP config and connection accept

	struct sockaddr_in client;
	unsigned int len = sizeof(client);
	int client_socket;
	char client_ip[INET_ADDRSTRLEN];
	int client_port;
	client_socket = accept(fd, (struct sockaddr *)&client, &len);
	if (client_socket < 0) {
	printf("No connection accepted\n");
	isRunning = false;
	} else {
	 printf("Client connected\n");
	}
	
	//getting a message
	char buf[512];
	memset(buf, 0, sizeof buf);
	while (recv(client_socket, buf, 511, 0) > 0) { 
	printf("\nMessage from client: \n%s\n", buf);

	send(client_socket, buf, 511, 0);
	}


	printf("End of the listening TCP loop\n");
	close(client_socket);
}

/* TO-DO: Add htons or other combination to convert from network byte order to host or vice versa */
int main(int argc, char *argv[])
{
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
		main_loop(socket_file_descriptor);
	}

	close(socket_file_descriptor);
	printf("It works\n");
	return 0;
}
