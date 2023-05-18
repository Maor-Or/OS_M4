
#include "st_reactor.hpp"
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <poll.h>
#include <iostream>

using namespace std;

typedef struct reactor_
{

    unordered_map<int, handler_t> *hashmap;
    struct pollfd *pfds;
    pthread_t r_thread;
    int pfds_index;
    int isStopReactor;

} Reactor, *Preactor;

void *threadFunction(void *reactor);

void *createReactor()
{

    // allocating memory for the struct
    Preactor reactor = (Preactor)calloc(1, sizeof(Reactor));

    // if arr is null then we return null cause there was an error
    if (!reactor)
        return NULL;

    // initilizing the attributes
    reactor->pfds = (struct pollfd *)malloc(sizeof(struct pollfd) * 1);
    reactor->pfds_index = 0;
    reactor->isStopReactor = 0;
    printf("before initing hashmap\n");
    reactor->hashmap = new unordered_map<int, handler_t>;
    printf("after initing hashmap\n");

    // returns the reactor:
    return reactor;
}

void addFd(void *reactor, int fd, handler_t handler)
{
    printf("reached addFd, fd: %d, handler: %p\n", fd, handler);
    // adding to the hashmap:
    struct reactor_ *react = (Preactor)reactor;
    // unordered_map<int, handler_t> hashmap = {};
    // hashmap[fd] = handler;
    // TODO: added this:
    printf("react\n");
    // react->hashmap.emplace(fd, handler);
    unordered_map<int, handler_t> *hashmap = react->hashmap;
    (*hashmap)[fd] = handler;
    // react->hashmap[fd] = handler;
    printf("reached hashmapAdd\n");

    // adding to the pfds:
    react->pfds[react->pfds_index].fd = fd;
    react->pfds[react->pfds_index++].events = POLLIN;

    // reallocating the data for a bigger array:
    react->pfds = (struct pollfd *)realloc(react->pfds, sizeof(struct pollfd) * (react->pfds_index + 1));
}

void startReactor(void *reactor)
{
    struct reactor_ *react = (Preactor)reactor;
    react->isStopReactor = 0;

    int thread_create_result = pthread_create(&(react->r_thread), NULL, threadFunction, reactor);
    if (thread_create_result != 0)
    {
        printf("Failed to create thread.\n");
        exit(1);
    }
}

void stopReactor(void *reactor)
{
    struct reactor_ *react = (Preactor)reactor;
    react->isStopReactor = 1;
}

void WaitFor(void *reactor)
{
    struct reactor_ *react = (Preactor)reactor;
    pthread_join(react->r_thread, NULL);
}

// Function to be executed in the thread
void *threadFunction(void *reactor)
{

    struct reactor_ *react = (Preactor)reactor;
    react->isStopReactor = 0;
    while (1)
    {
        if (react->isStopReactor)
        {
            break;
        }

        int poll_count = poll(react->pfds, react->pfds_index, -1);
        if (poll_count < 0)
        {
            perror("Erorr in poll");
            exit(1);
        }

        // going through all fds, looking for the hot ones
        for (int i = 0; i < react->pfds_index; i++)
        {
            if (react->pfds[i].revents & POLLIN)
            {
                // handler_t currFunc = react->hashmap[react->pfds[i].fd];

                // TODO: added this
                unordered_map<int, handler_t> *hashmap = react->hashmap;
                handler_t currFunc = (*hashmap).at (react->pfds[i].fd);
                // handler_t currFunc = react->hashmap.at(react->pfds[i].fd);
                currFunc(react->pfds[i].fd, reactor);
            }
        }
    }
    return NULL;
}