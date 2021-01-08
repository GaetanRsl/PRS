all : main

main : Server1.o ServerProject.o MultiClients.o Server2.o 
	gcc -Wall ServerProject.o -o ServerProject
	gcc -Wall Server1.o -o Server1
	gcc -Wall MultiClients.o -o MultiClients
	gcc -Wall Server2.o -o Server2


ServerProject.o : ServerProject.c
	gcc -Wall -c ServerProject.c -o ServerProject.o

Server1.o : Server1.c 
	gcc -Wall -c Server1.c -o Server1.o

MultiClients.o : MultiClients.c
	gcc -Wall -c MultiClients.c -o MultiClients.o

Server2.o : Server2.c
	gcc -Wall -c Server2.c -o Server2.o

clean:
	rm -f *.o