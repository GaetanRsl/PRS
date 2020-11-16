#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {

    struct sockaddr_in address;
    int addr = inet_aton(argv[1], &address.sin_addr);
    //int port =argv[2];
    address.sin_family = AF_INET;
    address.sin_port = htons(atoi(argv[2]));
    //char msg[]="toto";
    char msg[512];
    char buffer[512];

    if (argc != 3){
        printf("Nombre d'arguments invalide \n");
        printf("syntaxe : ./client <ip_serveur> <port serveur> \n");
    }

    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket <0){
        printf("clientsocket -1");
        return -1;
    }
    printf("inet = %d \n", addr);

    int connection = connect(clientSocket,(struct sockaddr *) &address, sizeof(address));

    if(connection<0){
        printf("connection problem\n");
        return -1;
    }
    printf("connection etablit\n");
    int cont =1;
    while(cont){       
        fgets(msg, 512, stdin);
        write(clientSocket, msg, sizeof(msg));
        read(clientSocket, buffer, sizeof(buffer));
        printf(" Message serveur TCP : %s\n",buffer);
        if (strcmp(msg,"stop") == 0) {
            cont= 0; 
        }
    }
    close(clientSocket);

return(0);
}