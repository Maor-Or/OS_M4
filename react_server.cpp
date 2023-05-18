#include "st_reactor.hpp"

/*
** pollserver.c -- a cheezy multiperson chat server
*/

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

// // Return a listening socket
// int get_listener_socket(void)
// {
//     int listener; // Listening socket descriptor
//     int yes = 1;  // For setsockopt() SO_REUSEADDR, below
//     int rv;

//     struct addrinfo hints, *ai, *p;

//     // Get us a socket and bind it
//     memset(&hints, 0, sizeof hints);
//     hints.ai_family = AF_UNSPEC;
//     hints.ai_socktype = SOCK_STREAM;
//     hints.ai_flags = AI_PASSIVE;
//     if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0)
//     {
//         fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
//         exit(1);
//     }

//     for (p = ai; p != NULL; p = p->ai_next)
//     {
//         listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
//         if (listener < 0)
//         {
//             continue;
//         }

//         // Lose the pesky "address already in use" error message
//         setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

//         if (bind(listener, p->ai_addr, p->ai_addrlen) < 0)
//         {
//             close(listener);
//             continue;
//         }

//         break;
//     }

//     freeaddrinfo(ai); // All done with this

//     // If we got here, it means we didn't get bound
//     if (p == NULL)
//     {
//         return -1;
//     }

//     // Listen
//     if (listen(listener, 10) == -1)
//     {
//         return -1;
//     }

//     return listener;
// }

// Add a new file descriptor to the set
void add_to_pfds(struct pollfd *pfds[], int newfd, int *fd_count, int *fd_size)
{
    // If we don't have room, add more space in the pfds array
    if (*fd_count == *fd_size)
    {
        *fd_size *= 2; // Double it

        *pfds = (struct pollfd *)realloc(*pfds, sizeof(**pfds) * (*fd_size));
    }

    (*pfds)[*fd_count].fd = newfd;
    (*pfds)[*fd_count].events = POLLIN; // Check ready-to-read

    (*fd_count)++;
}

// Remove an index from the set
void del_from_pfds(struct pollfd pfds[], int i, int *fd_count)
{
    // Copy the one from the end over this one
    pfds[i] = pfds[*fd_count - 1];

    (*fd_count)--;
}

// function for any chatter
void chat_func(int chatter_fd, void *reactor)
{
    char buffer[250] = {'\0'};

    recv(chatter_fd, buffer, 250, 0);
    printf("Received message: %s", buffer);
}

// listener_func for adding new sock_fds to the reactor
void listener_func(int listener_fd, void *reactor)
{
    struct sockaddr_storage remoteaddr; // Client address

    socklen_t addrlen = sizeof(remoteaddr);
    int newfd = accept(listener_fd,
                       (struct sockaddr *)&remoteaddr,
                       &addrlen);

    addFd((Preactor)reactor, newfd, chat_func);
}

// Main
int main(void)
{
    Preactor reactor = (Preactor)createReactor();

    // int listener; // Listening socket descriptor

    // // Set up and get a listening socket
    // listener = get_listener_socket();

    // if (listener == -1)
    // {
    //     fprintf(stderr, "error getting listening socket\n");
    //     exit(1);
    // }

    // Create socket
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0)
    {
        perror("socket");
        exit(1);
    }

    // Bind socket to address and port
    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // any IP at this port (Address to accept any incoming messages)

    server_addr.sin_port = htons(PORT);

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



    addFd(reactor, server_sock, listener_func);

    startReactor(reactor);

    WaitFor(reactor);

    // // Listen for incoming connections
    // if (listen(server_sock, 1) < 0)
    // {
    //     perror("listen");
    //     exit(1);
    // }

    // // Accept incoming connections
    // struct sockaddr_in client_addr = {0};
    // socklen_t client_addr_len = sizeof(client_addr);
    // int client_fds[1] = {0}; // array to hold connected client sockets
    // int num_clients = 0;

    // // activity on server socket file descriptor
    // int client_fd = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_len);
    // if (client_fd < 0)
    // {
    //     perror("erorr accept");
    //     exit(1);
    // }


    // // Main loop
    // for (;;)
    // {
    //     int poll_count = poll(pfds, fd_count, -1);

    //     if (poll_count == -1)
    //     {
    //         perror("poll");
    //         exit(1);
    //     }

    //     // Run through the existing connections looking for data to read
    //     for (int i = 0; i < fd_count; i++)
    //     {

    //         // Check if someone's ready to read
    //         if (pfds[i].revents & POLLIN)
    //         { // We got one!!

    //             if (pfds[i].fd == listener)
    //             {
    //                 // If listener is ready to read, handle new connection

    //                 addrlen = sizeof remoteaddr;
    //                 newfd = accept(listener,
    //                                (struct sockaddr *)&remoteaddr,
    //                                &addrlen);

    //                 if (newfd == -1)
    //                 {
    //                     perror("accept");
    //                 }
    //                 else
    //                 {
    //                     add_to_pfds(&pfds, newfd, &fd_count, &fd_size);

    //                     printf("pollserver: new connection from %s on "
    //                            "socket %d\n",
    //                            inet_ntop(remoteaddr.ss_family,
    //                                      get_in_addr((struct sockaddr *)&remoteaddr),
    //                                      remoteIP, INET6_ADDRSTRLEN),
    //                            newfd);
    //                 }
    //             }
    //             else
    //             {
    //                 // If not the listener, we're just a regular client
    //                 int nbytes = recv(pfds[i].fd, buf, sizeof buf, 0);

    //                 int sender_fd = pfds[i].fd;

    //                 if (nbytes <= 0)
    //                 {
    //                     // Got error or connection closed by client
    //                     if (nbytes == 0)
    //                     {
    //                         // Connection closed
    //                         printf("pollserver: socket %d hung up\n", sender_fd);
    //                     }
    //                     else
    //                     {
    //                         perror("recv");
    //                     }

    //                     close(pfds[i].fd); // Bye!

    //                     del_from_pfds(pfds, i, &fd_count);
    //                 }
    //                 else
    //                 {
    //                     // We got some good data from a client

    //                     for (int j = 0; j < fd_count; j++)
    //                     {
    //                         // Send to everyone!
    //                         int dest_fd = pfds[j].fd;

    //                         // Except the listener and ourselves
    //                         if (dest_fd != listener && dest_fd != sender_fd)
    //                         {
    //                             if (send(dest_fd, buf, nbytes, 0) == -1)
    //                             {
    //                                 perror("send");
    //                             }
    //                         }
    //                     }
    //                 }
    //             } // END handle data from client
    //         }     // END got ready-to-read from poll()
    //     }         // END looping through file descriptors
    // }             // END for(;;)--and you thought it would never end!

    // return 0;
}