#pragma once
#include <unordered_map>


typedef void(*handler_t)(int, void*);


void* createReactor();
void stopReactor(void *reactor);
void startReactor(void * reactor);
void addFd(void * reactor,int fd, handler_t handler);
void WaitFor(void * reactor);

typedef struct reactor_ *Preactor;