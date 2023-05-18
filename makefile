
CC = clang++-14
FLAGS = -Wall

all: st_reactor.so #st_reactor.so

st_reactor.so:st_reactor.o
	$(CC) -shared -fPIC st_reactor.o -o st_reactor.so
# rm st_reactor.o

# st_reactor:st_reactor.o
# 	$(CC) $(FLAGS) -o st_reactor st_reactor.o



st_reactor.o:st_reactor.cpp st_reactor.hpp
	$(CC) $(FLAGS) -c st_reactor.cpp


.PHONY: clean all

clean:
	rm -f *.o *.so
