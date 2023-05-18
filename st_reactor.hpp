#pragma once
#include <unordered_map>


typedef void(*handler_t)();


void* createReactor();
void stopReactor(void *reactor);
void startReactor(void * reactor);
void addFd(void * reactor,int fd, handler_t handler);
void WaitFor(void * reactor);
