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

#include "c_datastructures.h"
#include "client.h"

#define HOST "127.0.0.1"
#define DEFAULT_PORT "9090"
#define BACKLOG 10 // how many pending connections queue holds

bool isRunning= true;

HashTable* chatrooms = NULL;
HashTable* clients = NULL;

int int_server(unsigned int);
void main_loop(int fd, Client* client_list[], HashTable* chatrooms);
int main(int argc, char* argv[]);
