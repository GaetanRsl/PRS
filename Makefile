all : main

main : client_UDP.o ServerProject.o MultiClients.o
	gcc -Wall ServerProject.o -o ServerProject
	gcc -Wall client_UDP.o -o client_UDP
	gcc -Wall MultiClients.o -o MultiClients

ServerProject.o : ServerProject.c
	gcc -Wall -c ServerProject.c -o ServerProject.o

client_UDP.o : client_UDP.c 
	gcc -Wall -c client_UDP.c -o client_UDP.o

MultiClients.o : MultiClients.c
	gcc -Wall -c MultiClients.c -o MultiClients.o

clean:
	rm -f *.o