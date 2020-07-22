.PHONY: all
all: node head

node: Node.o libduSocket.a 
	gcc Node.o -L. -lduSocket -o node

Node.o: Node.c duSocket.h
	gcc Node.c -c

head: Head.o libduSocket.a
	gcc -fopenmp Head.o -L.	-lduSocket -o head

Head.o: Head.c duSocket.h
	gcc -fopenmp Head.c -c

duSocket.o: duSocket.c
	gcc duSocket.c -c

libduSocket.a: duSocket.o 
	ar -cvq libduSocket.a duSocket.o 
libs: libduSocket.a

clean:
	rm -f node head *.o *.a *.gch 
