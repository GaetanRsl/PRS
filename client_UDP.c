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
    //char msg[512];
    char msg_SYN[]="SYN";
    char msg_ACK[]="ACK";
    char bufferFile[1035];
    char buffer[1024];
    char buffer_final[1024];
    char filename[10];
    strcpy(filename, argv[3]);
    //char revbuf[512];


    if (argc != 4){
        printf("Invalid number of arguments \n");
        printf("syntaxe : ./client <ip server> <port serveur> <filename>\n");
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
   
    
    recvfrom(clientSocketConnection,buffer, sizeof(buffer), 0, (struct sockaddr *)&address, &tailleAddr);
        //read(clientSocket, buffer, sizeof(buffer));
    char private_port[4];
    //GET PRIVATE PORT FROM SERVER
    memcpy(private_port, &buffer[7], 4);
    printf("port serveur echange de message : %s\n", private_port);
    address2.sin_port=htons(atoi(private_port));

    printf("Message Serveur UDP Connection : %s\n",buffer);
    if (strncmp(buffer, "SYN-ACK3456",11) == 0) {
            sendto(clientSocketConnection, (const char *)msg_ACK, sizeof(msg_ACK),0, (struct sockaddr *) &address, tailleAddr);
    }
    printf("fin du handshake\n");
   char file[15] = "udpfile.jpg";
   FILE *f = fopen(file, "w");
    int cont =1;
    int msgSize = 1;
    while(cont){
        
        if(cont == 1) {
            printf("You asked for : %c \n", filename);
            //sendto(clientSocket, (const char *)filename, sizeof(filename),0, (struct sockaddr *) &address2, tailleAddr2);
             cont +=1;
        }else{
            bzero(filename, 10);
            printf("Name of the file : %s", filename);
            fgets(filename, 10, stdin);           
        }

        sendto(clientSocket, (const char *)filename, sizeof(filename),0, (struct sockaddr *) &address2, tailleAddr2);
        recvfrom(clientSocket,buffer, sizeof(buffer), 0, (struct sockaddr *)&address2, &tailleAddr2);
        printf("Message from Server : %s", buffer);
        
        
        //printf("message received %s\n", bufferFile);
        //fwrite(bufferFile, sizeof(char), msgSize, f);
        
        int r = 1;
        char ack_file[6];
        while(r ==1){
            bzero(bufferFile, 1035);
            msgSize = recvfrom(clientSocket, bufferFile, sizeof(bufferFile), 0, (struct sockaddr*)&address2, &tailleAddr2);
            printf("Message size : %d\n", msgSize);
            if(strcmp(bufferFile, "END")!=0){
                bzero(ack_file, 6);
                memcpy(ack_file, bufferFile,6);
                printf("seq num : %s\n", ack_file);    
                int y=sendto(clientSocket, ack_file, sizeof(ack_file),0, (struct sockaddr *) &address2, tailleAddr2);
                
                //printf("%d\n",y);
                memcpy(buffer_final, bufferFile+6, 1024);
                fwrite(buffer_final, sizeof(char), 1024, f);
                bzero(buffer, 512);
            }else{
                r=0;
                fclose(f);
                break;
            }       
        }
       
    }
    
    close(clientSocketConnection);

return(0);
}