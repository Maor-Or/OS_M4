#include "uthash.h"

#include <glib.h>
#include "st_reactor.h"

typedef struct reactor_
{
GHashTable* hashtable;

}Reactor,*Preactor;


void* createReactor()
{

    //allocating memory for the struct
    Reactor reactor = (Preactor)calloc(1, sizeof(Reactor));
    
    //if arr is null then we return null cause there was an error
    if (!Arr)
        return NULL;
    
    //initilizing the attributes
    GHashTable* hashtable = g_hash_table_new(g_str_hash, g_str_equal);

    
    return Arr;
}

void startReactor(void * reactor)
{

}

void stopReactor(void *reactor)
{

}


void addFd(void * reactor,int fd, handler_t handler)
{
    reactor->
}

void WaitFor(void * reactor){}



int Server_Part_A(char port[6], int usePredefined)
{

    // Create socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0)
    {
        perror("socket");
        exit(1);
    }

    // Register the signal handler
    signal(SIGINT, handle_sigint);

    // Bind socket to address and port
    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // any IP at this port (Address to accept any incoming messages)

    // else - part A, use the user inputed params
    else
    {
        server_addr.sin_port = htons(atoi(port));
    }

    int optval = 1;
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
    {
        perror("Error setting socket option");
        return -1;
    }

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        exit(1);
    }

    // Listen for incoming connections
    if (listen(server_sock, 1) < 0)
    {
        perror("listen");
        exit(1);
    }

    // Accept incoming connections
    struct sockaddr_in client_addr = {0};
    socklen_t client_addr_len = sizeof(client_addr);
    int client_fds[1] = {0}; // array to hold connected client sockets
    int num_clients = 0;

    // activity on server socket file descriptor
    int client_fd = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_fd < 0)
    {
        perror("erorr accept");
        exit(1);
    }

    // Set up pollfd struct for server socket and stdin
    struct pollfd fds[2];
    fds[0].fd = client_fd;    // server socket file descriptor
    fds[0].events = POLLIN;   // wait for incoming data
    fds[1].fd = STDIN_FILENO; // keyboard input file descriptor
    fds[1].events = POLLIN;   // wait for incoming data

    printf("A new client connection accepted\n");
    while (1)
    {
        int ret = poll(fds, 2, -1); // -1 means wait indefinitely for activity
        if (ret < 0)
        {
            perror("Erorr in poll");
            exit(1);
        }

        // Check for incoming connections
        if (fds[0].revents & POLLIN)
        {
            char buffer[BUFFER_SIZE] = {0};
            ssize_t num_bytes = recv(client_fd, buffer, BUFFER_SIZE, 0);
            printf("Received from client: %s\n", buffer);
        }

        // Chcek for keyboard input
        if (fds[1].revents & POLLIN)
        { // activity on keyboard input file descriptor

            char keyb_buffer[BUFFER_SIZE] = {0};
            fgets(keyb_buffer, BUFFER_SIZE, stdin);         // read input from keyboard
            keyb_buffer[strcspn(keyb_buffer, "\n")] = '\0'; // remove newline
            send(client_fd, keyb_buffer, BUFFER_SIZE, 0);
        }
    }
}


