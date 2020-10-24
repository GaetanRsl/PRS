#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

int main(int argc, char *argv[]) {

    struct sockaddr_in client_UDPConnection, client_UDP;

    struct sockaddr_in address_UDP, address_UDPConnection;
    memset((char*)&address_UDP, 0, sizeof(address_UDP));
    memset((char*)&address_UDPConnection, 0, sizeof(address_UDPConnection));

    int socketUDP = socket(AF_INET, SOCK_DGRAM, 0);
    printf("Socket UDP cree\n");
    int socketUDP_connection = socket(AF_INET, SOCK_DGRAM, 0);

    if (socketUDP_connection<0){
        printf("pb sck udp connect\n");
        return -1;
    }

    if (socketUDP <0){
        printf("pb socket UDP\n");
        return -1;
    }
    address_UDP.sin_addr.s_addr = htonl(INADDR_ANY);
    address_UDP.sin_family = AF_INET;
    address_UDP.sin_port=htons(atoi(argv[2]));

    address_UDPConnection.sin_addr.s_addr = htonl(INADDR_ANY);
    address_UDPConnection.sin_family = AF_INET;
    address_UDPConnection.sin_port=htons(atoi(argv[1]));


    char buffer_UDP[10];
    char msg_SYNACK[]="SYN-ACK";
    char buffer_UDPConnection[10];
    char port[6];
    strcpy(port, argv[2]);
    char end[10]="END";

    fd_set fd;
    FD_ZERO(&fd);

    if (argc != 3){
        printf("Nombre d'arguments invalide \n");
        printf("syntaxe : ./serveur <port serveur UDP Connection> <UDP message>\n");
        return -1;
    }

    int valid = 1;
    setsockopt(socketUDP, SOL_SOCKET, SO_REUSEADDR, &valid, sizeof(int));

    int bind2 = bind(socketUDP_connection, (struct sockaddr *)&address_UDPConnection, sizeof(address_UDPConnection));
    if (bind2<0){
        printf("problem socket UDP Connection\n");
        return -1;
    }
    int t = bind(socketUDP, (struct sockaddr *)&address_UDP, sizeof(address_UDP));
    if (t<0){
        printf("problem socket UDP\n");
        return -1;
    }

    struct sockaddr init_udpConection, init_udp;
    socklen_t v = sizeof(init_udpConection);
    socklen_t b = sizeof(init_udp);

    // Declaration des variables pour les fichiers
    char fs_name[10] = "theo.jpg";
    char sdbuf[1024];
    FILE *fs = fopen(fs_name, "r");


    while(1){
        printf("Waiting for message... \n");
        FD_SET(socketUDP_connection, &fd);
        FD_SET(socketUDP, &fd);

        int slct = select(socketUDP_connection+1, &fd, NULL, NULL, NULL);
        
        if(slct <0){
            printf("Error Select \n");
            return -1;
        }
        if(FD_ISSET(socketUDP_connection, &fd) != 0){
            printf("Beginning of the connection\n");
            recvfrom(socketUDP_connection, &buffer_UDPConnection, sizeof(buffer_UDPConnection),0, (struct sockaddr *)&client_UDPConnection, &v);
            printf("First message from UDP Client : %s\n", buffer_UDPConnection);

            if(strncmp(buffer_UDPConnection, "SYN",3) ==0){
                sendto(socketUDP_connection, (const char*)&msg_SYNACK, sizeof(msg_SYNACK),0, (struct sockaddr *)&client_UDPConnection, v);           
            }
            recvfrom(socketUDP_connection, &buffer_UDPConnection, sizeof(buffer_UDPConnection),0, (struct sockaddr *)&client_UDPConnection, &v);
            printf("Second message from client : %s \n", buffer_UDPConnection);
            if(strncmp(buffer_UDPConnection, "ACK",3) ==0){
            //sendto(socketUDP, (const char*)&msg_SYNACK, sizeof(msg_SYNACK),0, (struct sockaddr *)&client_UDP, v);     
                printf("Succesful 3 way handshake\n");
                sendto(socketUDP_connection, port, sizeof(port),0, (struct sockaddr *)&client_UDPConnection, v);           
            }
            

        }else if(FD_ISSET(socketUDP, &fd)!=0)
        {
            printf("ENTER FILE TRANSFERT\n");
            
            int recv = recvfrom(socketUDP, buffer_UDP, sizeof(buffer_UDP),0, (struct sockaddr *)&client_UDP, &b);
            printf("port : %d\n", ntohs(client_UDP.sin_port));
            printf("Message client UDP : %s\n", buffer_UDP);
            sendto(socketUDP, buffer_UDP, recv,0, (struct sockaddr *)&client_UDP, b);
            //printf("apres send to\n");
            if (fs == NULL){
                printf("Error, file %s not found \n", fs_name);
                exit(1);
            }
            int fs_block_sz = 0;
            bzero(sdbuf, 1024);
            while((fs_block_sz = fread(sdbuf, sizeof(char), 1024,fs)) > 0 ){
                //printf("Dans le while\n");
                //int taille = fread(sdbuf, sizeof(char), 1024, fs);
                //printf("taille fread : %d", taille);
                int s =sendto(socketUDP, sdbuf, 1024, 0, (struct sockaddr *)&client_UDP, b);
                //printf("Valeur send to : %d\n", s);
            }
            
            printf("contenu du buffer : %s\n", sdbuf);
            //ENVOI DU "END"
            sendto(socketUDP, (const char*)&end, sizeof(end), 0, (struct sockaddr *)&client_UDP, b);

            printf("File send correctly \n");           
            
        }        
    }
   
return (0);
}
