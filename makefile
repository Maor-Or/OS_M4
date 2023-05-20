CC = clang++-14
FLAGS = -Wall -g

all:  react_server st_reactor.so

st_reactor.so: st_reactor.cpp st_reactor.hpp
	$(CC) -shared -fPIC st_reactor.cpp -o st_reactor.so -pthread


react_server: react_server.cpp st_reactor.hpp st_reactor.so
	$(CC) $(FLAGS) react_server.cpp -o react_server ./st_reactor.so -pthread

.PHONY: clean all

clean:
	rm -f *.o *.so react_server