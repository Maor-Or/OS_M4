
#pragma once

typedef void(*handler_t)(char funcname[10]);

// #include "handler.h"
// #define REACTOR

// #endif

void* createReactor();
void stopReactor(void *reactor);
void startReactor(void * reactor);
void addFd(void * reactor,int fd, handler_t handler);
void WaitFor(void * reactor);
