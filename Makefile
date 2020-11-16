all : main

main : client_UDP.o ServerProject.o
	gcc -Wall ServerProject.o -o ServerProject
	gcc -Wall client_UDP.o -o client_UDP

ServerProject.o : ServerProject.c
	gcc -Wall -c ServerProject.c -o ServerProject.o

client_UDP.o : client_UDP.c 
	gcc -Wall -c client_UDP.c -o client_UDP.o

clean:
	rm -f *.o