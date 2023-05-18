
CC = gcc
FLAGS = -Wall

all: st_reactor.so

st_reactor.so:st_reactor.o
	$(CC) -shared -fPIC st_reactor.o -o st_reactor.so pkg-config --cflags --libs glib-2.0

st_reactor.o:st_reactor.c st_reactor.h
	$(CC) $(FLAGS) -c st_reactor.c


.PHONY: clean all

clean:
	rm -f *.o *.so
