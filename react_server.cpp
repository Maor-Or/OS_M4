#include "st_reactor.hpp"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>

#define PORT 9034 // Port we're listening on

// global:
int server_socket = 0;

// Signal handler function for SIGINT
void handle_sigint(int signum)
{
    // caught the ctrl c
    printf("closing socket\n");
    if (server_socket != 0)
        close(server_socket);
    exit(0);
}

// function for any chatter
void chat_func(int chatter_fd, void *reactor)
{
    // Register the signal handler
    signal(SIGINT, handle_sigint);

    char buffer[250] = {'\0'};

    recv(chatter_fd, buffer, 250, 0);
    
    //incase a disconnect signal was sent, the buffer is empty:
    if (strlen(buffer) == 0)
    {
        close(chatter_fd);
    }
    else
    {
        printf("Received message: %s\n", buffer);
    }
}

// listener_func for adding new sock_fds to the reactor
void listener_func(int listener_fd, void *reactor)
{
    // Register the signal handler
    signal(SIGINT, handle_sigint);

    struct sockaddr_in client_address;
    socklen_t client_address_length = sizeof(client_address);

    int newfd = accept(listener_fd, (struct sockaddr *)&client_address, &client_address_length);
    if (newfd == -1)
    {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }

    addFd((Preactor)reactor, newfd, chat_func);
}

// Main
int main(void)
{
    // Register the signal handler
    signal(SIGINT, handle_sigint);

    Preactor reactor = (Preactor)createReactor();

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options to allow reusing the address immediately after closing
    int reuse = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1)
    {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    // Bind socket to address and port
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 5) == -1)
    {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    // Define the handler function pointer
    handler_t handler = &listener_func;

    // Call addFd with the appropriate arguments
    addFd(reactor, server_socket, handler);

    startReactor(reactor);

    WaitFor(reactor);

    return 0;
}