
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





////////////////////////////////////////////

#include <glib.h>

#include <stdio.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>
#include <sys/time.h>
#include <openssl/md5.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define PARAM_SIZE 20

// TODO REDUCDE 1 SEC FROM TIME CALCUTION DUE TO SLEEP FUNCTION
#define BUFFER_SIZE 1024
#define CHUNK_SIZE 32768
#define FILE_SIZE 104857600 // for generating 100 MB in bytes

// includes for uds:
#include <sys/un.h>
#define SOCKET_PATH "/tmp/mysocket.sock" // socket file path

// pre-defined for chatting in part B
#define SERVER_IP_ADDRESS "127.0.0.1"
#define SERVER_PORT 9999

int server_sock = 0;
int client_sock = 0;
int q_flag = 0;

int Client_Part_A(char server_ip[32], char port[6], int usePredefined, char type[6], char param[PARAM_SIZE], char hash[MD5_DIGEST_LENGTH * 2 + 1]);
int Client_Part_B(char server_ip[32], char port[6], char type[6], char param[PARAM_SIZE]);
int Server_Part_A(char port[6], int usePredefined);

int ipv4_or_6_send_tcp(int iptype, char *p_data, char server_ip[32], char port[6]); // 0 - ipv4, 1 - ipv6
int ipv4_or_6_send_udp(int iptype, char *p_data, char server_ip[32], char port[6]); // 0 - ipv4, 1 - ipv6
int uds_send_dgram(char *p_data);
int uds_send_stream(char *p_data);
int mmap_send(char *p_data, char param[PARAM_SIZE]);
int pipe_send(char *p_data, char param[PARAM_SIZE]);
int ipv4_or_6_receive_tcp(int iptype, char hash[MD5_DIGEST_LENGTH * 2 + 1], char time_string[32], char port[6]); // 0 - ipv4, 1 - ipv6
int ipv4_or_6_receive_udp(int iptype, char hash[MD5_DIGEST_LENGTH * 2 + 1], char time_string[32], char port[6]); // 0 - ipv4, 1 - ipv6
int uds_receive_dgram(char hash[MD5_DIGEST_LENGTH * 2 + 1], char time_string[32]);
int uds_receive_stream(char hash[MD5_DIGEST_LENGTH * 2 + 1], char time_string[32]);
int mmap_receive(char hash[MD5_DIGEST_LENGTH * 2 + 1], char time_string[32], char param[PARAM_SIZE]);
int pipe_receive(char hash[MD5_DIGEST_LENGTH * 2 + 1], char time_string[32], char param[PARAM_SIZE]);
void Print_Reuslts(int q_flag, char hash[MD5_DIGEST_LENGTH * 2 + 1], long long time_result, char *type_param);
// making the hash into string
void md5_hash_string(const char *input, char *output)
{
    unsigned char hash[MD5_DIGEST_LENGTH];
    MD5((const unsigned char *)input, strlen(input), hash);

    for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
    {
        sprintf(&output[i * 2], "%02x", hash[i]);
    }
}

// Signal handler function for SIGINT
void handle_sigint(int signum)
{
    // caught the ctrl c
    printf("closing socket\n");
    if (server_sock != 0)
        close(server_sock);
    else if (client_sock != 0)
        close(client_sock);

    exit(0);
}

int main(int argc, char *argv[]) // stnc -c IP PORT -p <type> <param>
{                                // 0   1  2   3   4    5      6
    if (argc > 2)
    {
        if (strcmp(argv[1], "-c") == 0) // in client side
        {
            // Part A:
            if (argc == 4) // stnc -c IP PORT
            {              // TODO: CHECK IF WE NEED TO CHECK IF THE PORT AND IP ARE VALID
                char server_ip[32];
                char port[6];
                strcpy(server_ip, argv[2]);
                strcpy(port, argv[3]);
                Client_Part_A(server_ip, port, 0, NULL, NULL, NULL);
            }
            // Part B
            else if (argc == 7) // stnc -c IP PORT -p <type> <param>
            {
                char server_ip[32];
                char type_port[6];
                char type[6];
                char param[PARAM_SIZE];
                strcpy(server_ip, argv[2]);
                strcpy(type_port, argv[3]);
                strcpy(type, argv[5]);
                strcpy(param, argv[6]);
                Client_Part_B(server_ip, type_port, type, param);
            }
        }
        // server side:
        else if (strcmp(argv[1], "-s") == 0)
        {
            // Part A
            if (argc == 3) // stnc -s PORT
            {
                char port[6];
                strcpy(port, argv[2]);
                Server_Part_A(port, 0);
            }
            // Part B
            // stnc -s port -p (p for performance test) -q (q for quiet)
            if (argc == 5 || argc == 4)
            {
                char port[6];
                strcpy(port, argv[2]);
                if (argc == 5 && (strcmp(argv[4], "-q") == 0))
                {
                    q_flag = 1;
                }
                Server_Part_A(port, 1);
            }
        }
        else
        {
            printf("Error");
            exit(1);
        }
    }
    else
    {
        printf("Error");
        exit(1);
    }

    return 0;
}

//-----------------------------------------------------------------------------------//

int Client_Part_A(char server_ip[32], char port[6], int usePredefined, char type[6], char param[PARAM_SIZE], char hash[MD5_DIGEST_LENGTH * 2 + 1])
{
    client_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (client_sock == -1)
    {
        printf("Could not create socket : %d", errno);
        return -1;
    }

    // Register the signal handler
    signal(SIGINT, handle_sigint);

    // "sockaddr_in" is the "derived" from sockaddr structure
    // used for IPv4 communication. For IPv6, use sockaddr_in6
    //
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;

    int rval;
    // if part B, use predefined params
    if (usePredefined)
    {
        serverAddress.sin_port = htons(SERVER_PORT);                                         // (5001 = 0x89 0x13) little endian => (0x13 0x89) network endian (big endian)
        rval = inet_pton(AF_INET, (const char *)SERVER_IP_ADDRESS, &serverAddress.sin_addr); // convert IPv4 and IPv6 addresses from text to binary form
    }
    // else - part A, use the user inputed params
    else
    {
        serverAddress.sin_port = htons(atoi(port));                                  // (5001 = 0x89 0x13) little endian => (0x13 0x89) network endian (big endian)
        rval = inet_pton(AF_INET, (const char *)server_ip, &serverAddress.sin_addr); // convert IPv4 and IPv6 addresses from text to binary form
    }
    // e.g. 127.0.0.1 => 0x7f000001 => 01111111.00000000.00000000.00000001 => 2130706433
    if (rval <= 0)
    {
        printf("inet_pton() failed");
        return -1;
    }

    // Make a connection to the server with socket SendingSocket.
    int connectResult = connect(client_sock, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (connectResult == -1)
    {
        printf("connect() failed with error code : %d", errno);
        // cleanup the socket;
        close(client_sock);
        return -1;
    }

    // Set up pollfd struct for client socket and stdin
    struct pollfd fds[2];
    fds[0].fd = client_sock;  // client socket file descriptor
    fds[0].events = POLLIN;   // wait for incoming data
    fds[1].fd = STDIN_FILENO; // keyboard input file descriptor
    fds[1].events = POLLIN;   // wait for incoming data

    // if part B, do the predefined communication
    if (usePredefined)
    {
        // sending type:
        // printf("sending type: %s \n", type);
        send(client_sock, type, 6 * sizeof(char), 0);

        // sending param:
        // printf("sending param: %s \n", param);
        send(client_sock, param, PARAM_SIZE * sizeof(char), 0);

        // sending port:
        // printf("sending port: %s \n", port);
        send(client_sock, port, 6 * sizeof(char), 0);

        // sending hash:
        // printf("sending hash: %s\n", hash);
        send(client_sock, hash, (MD5_DIGEST_LENGTH * 2 + 1) * sizeof(char), 0);

        // sending time:
        struct timeval tv;
        gettimeofday(&tv, NULL);
        long long milliseconds = tv.tv_sec * 1000LL + tv.tv_usec / 1000LL; // Convert to milliseconds
        char time_string[32];
        snprintf(time_string, sizeof(time_string), "%lld", milliseconds);
        send(client_sock, time_string, 32 * sizeof(char), 0);
    }
    // else - part A, normal chat mode
    else
    {
        while (1)
        {
            // Call poll() to wait for activity on file descriptors
            int ret = poll(fds, 2, -1); // -1 means wait indefinitely for activity
            if (ret < 0)
            {
                perror("poll");
                exit(1);
            }

            // Check for incoming data from server
            if (fds[0].revents & POLLIN)
            { // activity on client socket file descriptor
                char buffer[BUFFER_SIZE] = {0};
                ssize_t num_bytes = recv(client_sock, buffer, BUFFER_SIZE, 0);

                if (num_bytes < 0)
                {
                    perror("recv");
                    exit(1);
                }
                else if (num_bytes == 0)
                {
                    printf("Server closed connection\n");
                    exit(0);
                }
                else
                {
                    printf("Received from server: %s\n", buffer);
                }
            }

            // Check for keyboard input
            if (fds[1].revents & POLLIN)
            { // activity on keyboard input file descriptor
                char buffer[BUFFER_SIZE] = {0};
                fgets(buffer, BUFFER_SIZE, stdin);    // read input from keyboard
                buffer[strcspn(buffer, "\n")] = '\0'; // remove newline character
                // Send input to server
                send(client_sock, buffer, BUFFER_SIZE, 0);
            }
        }
    }
    return 0;
}

//-----------------------------------------------------------------------------------//

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
