#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {

    struct sockaddr_in address, address2;
    int addr = inet_aton(argv[1], &address.sin_addr);
    //int port =argv[2];
    address.sin_family = AF_INET;
    address.sin_port = htons(atoi(argv[2]));
    int addr2 = inet_aton(argv[1], &address2.sin_addr);
    address2.sin_family=AF_INET;
    
    //char msg[]="toto";
    char msg[512];
    char msg_SYN[]="SYN";
    char msg_ACK[]="ACK";
    char bufferFile[1024];
    char buffer[1024];
    //char revbuf[512];


    if (argc != 3){
        printf("Nombre d'arguments invalide \n");
        printf("syntaxe : ./client <ip_serveur> <port serveur> \n");
    }

    int clientSocketConnection = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSocketConnection <0){
        printf("clientsocket -1");
        return -1;
    }
    int clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    printf("inet = %d \n", addr);

    printf("connection etablit\n");
    
    socklen_t tailleAddr = sizeof(address);
    socklen_t tailleAddr2 = sizeof(address2);

    int y =sendto(clientSocketConnection, (const char *)msg_SYN, sizeof(msg_SYN),0, (struct sockaddr *) &address, tailleAddr);
        //write(clientSocket, msg, sizeof(msg));
    
    printf("after send %d %d %s\n", y, ntohs(address.sin_port), inet_ntoa(address.sin_addr));
    recvfrom(clientSocketConnection,buffer, sizeof(buffer), 0, (struct sockaddr *)&address, &tailleAddr);
        //read(clientSocket, buffer, sizeof(buffer));
    printf("Message Serveur UDP Connection : %s\n",buffer);
    if (strncmp(buffer, "SYN-ACK",7) == 0) {
            sendto(clientSocketConnection, (const char *)msg_ACK, sizeof(msg_ACK),0, (struct sockaddr *) &address, tailleAddr);
    }
    recvfrom(clientSocketConnection,buffer, sizeof(buffer), 0, (struct sockaddr *)&address, &tailleAddr);
    printf("port serveur echange de message : %s\n", buffer);
    address2.sin_port=htons(atoi(buffer));
    
   char file[15] = "udpfile.jpg";
   FILE *f = fopen(file, "w");
    int cont =1;
    int msgSize = 1;
    while(cont){
        
        printf("ENTER FILE NAME \n");
        fgets(msg, 512, stdin);
        //printf("message : %s", msg);
        sendto(clientSocket, (const char *)msg, sizeof(msg),0, (struct sockaddr *) &address2, tailleAddr2);
        recvfrom(clientSocket,buffer, sizeof(buffer), 0, (struct sockaddr *)&address2, &tailleAddr2);
        printf("Message from Server : %s", buffer);
        
        
        //printf("message received %s\n", bufferFile);
        //fwrite(bufferFile, sizeof(char), msgSize, f);
        
        int r = 1;
        while(r ==1){
            bzero(bufferFile, 1024);
            msgSize = recvfrom(clientSocket, bufferFile, sizeof(bufferFile), 0, (struct sockaddr*)&address2, &tailleAddr2);
            if(strcmp(bufferFile, "END")!=0){
                printf("dans le if\n");
                fwrite(bufferFile, sizeof(char), msgSize, f);
                bzero(buffer, 512);
            }else{
                r=0;
                fclose(f);
                break;
            }       
        }
        
        /*
        if (fr==NULL){
            printf("File %s Cannot be opened", fr_name);
        }
        //bzero(revbuf, 512);
        int fr_block_sz = 0;
        while((fr_block_sz = recvfrom(clientSocket, bufferFile, sizeof(bufferFile),0,(struct sockaddr *)&address2, &tailleAddr2) > 0)){
            int write_sz = fwrite(bufferFile, sizeof(char), fr_block_sz, fr);
            if (write_sz < fr_block_sz){
                error("File write failed \n");
            }
        }
        
        */      
        //fgets(msg, 512, stdin);
        //printf("message : %s", msg);
        //sendto(clientSocket, (const char *)msg, sizeof(msg),0, (struct sockaddr *) &address2, tailleAddr2)
    }
    
    close(clientSocketConnection);

return(0);
}