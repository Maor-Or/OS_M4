CC = clang++-14
FLAGS = -Wall

all: st_reactor.so react_server

st_reactor.so:st_reactor.hpp
	$(CC) -shared -fPIC st_reactor.cpp -o st_reactor.so -pthread
# rm st_reactor.o

# st_reactor:st_reactor.o
# 	$(CC) $(FLAGS) -o st_reactor st_reactor.o



# st_reactor.o:st_reactor.cpp st_reactor.hpp
# 	$(CC) $(FLAGS) -c st_reactor.cpp


react_server: st_reactor.hpp st_reactor.so
	$(CC) $(FLAGS) react_server.cpp -o react_server ./st_reactor.so -pthread

.PHONY: clean all

clean:
	rm -f *.o *.so react_server 