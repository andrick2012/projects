.PHONY: all

all: loader tester

loader: simLoader.o libSimHeader.a
	gcc -fopenmp simLoader.o -L. -lSimHeader -o loader

simLoader.o: simLoader.c simHeader.h
	gcc -fopenmp simLoader.c -c
	
tester: simTester.o libSimHeader.a
	gcc simTester.o -L. -lSimHeader -o tester
	
simTester.o: simTester.c simHeader.h
	gcc simTester.c -c
	
simHeader.o: simHeader.c
	gcc simHeader.c -c
	
libSimHeader.a: simHeader.o
	ar -cvq libSimHeader.a simHeader.o
	
libs: libSimHeader.a
	
clean:
	rm -f loader tester *.o *.a 
	