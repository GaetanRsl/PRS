#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
//#include <arpa/inet.h>
#include <sys/select.h>

int main(int argc, char *argv[]) {

    struct sockaddr_in client_UDP;
    struct sockaddr_in address_TCP;
    memset((char*)&address_TCP, 0, sizeof(address_TCP));

    struct sockaddr_in address_UDP;
    memset((char*)&address_UDP, 0, sizeof(address_UDP));

    int socketUDP = socket(AF_INET, SOCK_DGRAM, 0);

    if (socketUDP <0){
        return -1;
    }

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    printf("%d\n", serverSocket);
    char buffer[512];
    char buffer_UDP[512];

    fd_set fd;
    FD_ZERO(&fd);

    if (argc != 3){
        printf("Nombre d'arguments invalide \n");
        printf("syntaxe : ./serveur <port serveur TCP>  <UDP>\n");
        return -1;
    }


    if (serverSocket <0){
        return -1;
    }

    address_TCP.sin_addr.s_addr = htonl(INADDR_ANY);
    address_TCP.sin_family = AF_INET;
    address_TCP.sin_port=htons(atoi(argv[1]));

    address_UDP.sin_addr.s_addr = htonl(INADDR_ANY);
    address_UDP.sin_family = AF_INET;
    address_UDP.sin_port=htons(atoi(argv[2]));

    int t = bind(socketUDP, (struct sockaddr *)&address_UDP, sizeof(address_UDP));
    if (t<0){
        printf("problem socket UDP");
        return -1;
    }
    int valid =1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &valid, sizeof(int));
    int x = bind(serverSocket, (struct sockaddr *) &address_TCP,sizeof(address_TCP));
    if (x<0){
        printf("problem socket TCP\n");
    } 

    if (listen(serverSocket,1)<0){
        printf("problem listen \n");
        return -1;
    }
    struct sockaddr init;
    socklen_t c = sizeof(init);

    struct sockaddr init_udp;
    socklen_t v = sizeof(init_udp);

            //utilisation de select()

    //struct timeval tv;
 
        
    while(1){
        printf("Avant FD SET \n");
        FD_SET(serverSocket, &fd);
        FD_SET(socketUDP, &fd);
        //tv.tv_sec = 5;
        //tv.tv_usec = 0;
        
        //Creation des processus fils
        printf("entree du select\n");
        int slct = select(serverSocket+1, &fd, NULL, NULL, NULL);
        printf("sorti du select\n");
        if(slct <0){
            printf("Error Select \n");
            return -1;
        }
        printf("select\n");
        if (FD_ISSET(serverSocket, &fd) !=0){
            printf("Accepting\n");
            int client_desc = accept(serverSocket, (struct sockaddr *) &init, &c);
            printf("id socket client TCP : %d\n", client_desc);
            printf("ENTER TCP MODE\n");
            int pid = fork();

            if (pid == 0){
                close(serverSocket);
                printf("socket 1 %d\n", serverSocket);
                printf("socket desc %d \n", client_desc);
                //printf("message client : %s\n", buffer);
                int msg = read(client_desc, buffer, sizeof(buffer));
     
                printf("message client : %s\n", buffer);

                while(msg > 0){
                    write(client_desc, buffer, msg);
                    memset(buffer,0,512);
                    msg = read(client_desc, buffer, sizeof(buffer));
                    printf("message client TCP : %s\n", buffer);
                }
                close(client_desc);
                exit(0);
            }
            if (pid >0){
                printf("pid pere %d", pid);
                close(client_desc);
            }
        //close(client_desc);          
        }else if(FD_ISSET(socketUDP, &fd) !=0){
            printf("ENTER UDP MODE\n");
            
            recvfrom(socketUDP, &buffer_UDP, sizeof(buffer_UDP),0, (struct sockaddr *)&client_UDP, &v);
            printf("Message client UDP : %s\n", buffer_UDP);
            sendto(socketUDP, (const char*)&buffer_UDP, sizeof(buffer_UDP),0, (struct sockaddr *)&client_UDP, v);           
        }
        
    }
    
return (0);
}
