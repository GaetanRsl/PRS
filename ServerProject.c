#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>


struct elem{
    char seq[1024];
};
int main(int argc, char *argv[]) {

    

    int port_prive = 3456;
    
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
    address_UDP.sin_port=htons(port_prive);

    address_UDPConnection.sin_addr.s_addr = htonl(INADDR_ANY);
    address_UDPConnection.sin_family = AF_INET;
    address_UDPConnection.sin_port=htons(atoi(argv[1]));

    //char tableau_seq[9999][1024];
    char buffer_UDP[10];
    char buffer_ACK[10];
    char buffer_seq[6];
    char buffer_data_seq[1035];
    char msg_SYNACK[]="SYN-ACK3456";
    char buffer_UDPConnection[10];
    char buffer_total[1024];
    char fs_name[10];
    //char port[6];
    //strcpy(port, argv[2]);
    char end[10]="END";

    fd_set fd;
    FD_ZERO(&fd);

    if (argc != 2){
        printf("Nombre d'arguments invalide \n");
        printf("syntaxe : ./serveur <port serveur UDP Connection>\n");
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

    /*
    Declaration des ;variables pour les fichiers
    char fs_name[10] = "theo.jpg";
    char sdbuf[1024];
    FILE *fs = fopen(fs_name, "r");
    */
    struct timeval t0, t1;

    struct elem* tableau_seq=malloc(sizeof(struct elem )* 10000);
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
            //SYN RCV
            recvfrom(socketUDP_connection, &buffer_UDPConnection, sizeof(buffer_UDPConnection),0, (struct sockaddr *)&client_UDPConnection, &v);
            printf("First message from UDP Client : %s\n", buffer_UDPConnection);

            if(strncmp(buffer_UDPConnection, "SYN",3) ==0){
                //SYN-ACK SEND
                sendto(socketUDP_connection, (const char*)&msg_SYNACK, sizeof(msg_SYNACK),0, (struct sockaddr *)&client_UDPConnection, v);           
            }
            //ACK RCV
            recvfrom(socketUDP_connection, &buffer_UDPConnection, sizeof(buffer_UDPConnection),0, (struct sockaddr *)&client_UDPConnection, &v);
            printf("Second message from client : %s \n", buffer_UDPConnection);
            if(strncmp(buffer_UDPConnection, "ACK",3) ==0){
            //sendto(socketUDP, (const char*)&msg_SYNACK, sizeof(msg_SYNACK),0, (struct sockaddr *)&client_UDP, v);     
                printf("Succesful 3 way handshake\n");
                //Send private port number
                //sendto(socketUDP_connection, port, sizeof(port),0, (struct sockaddr *)&client_UDPConnection, v);           
            }else{
                EXIT_FAILURE;
            }
            

        }else if(FD_ISSET(socketUDP, &fd)!=0){

            printf("ENTER FILE TRANSFERT\n");
            
            int recv = recvfrom(socketUDP, buffer_UDP, sizeof(buffer_UDP),0, (struct sockaddr *)&client_UDP, &b);
            int file_sz = sizeof(buffer_UDP);
            memcpy(fs_name, buffer_UDP, file_sz);
            char sdbuf[1024];
            FILE *fs = fopen(fs_name, "r");

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
            int numSequence = 0;
            int timer0;
            int timer1;
            double moyenne = 0;
            struct timeval timeout = {1, 0};
            //int window_size = 1;
            //int dernier_paquet = 119; 
            //while(window_size < dernier_paquet){
                
                while((fs_block_sz = fread(sdbuf, sizeof(char), 1024,fs)) > 0 ){
                    //printf("avant\n");
                    memcpy(tableau_seq[numSequence].seq, sdbuf,1024);
                    //printf("apres\n");
                    sprintf(buffer_seq,"%d", numSequence);
                    memcpy(buffer_data_seq, buffer_seq, 6);
                    memcpy(buffer_data_seq + 6,sdbuf, 1024);



                    int send = 0;
                    while(send ==0){
                        sendto(socketUDP, buffer_data_seq, 1035, 0, (struct sockaddr *)&client_UDP, b);
                        FD_SET(socketUDP, &fd);
                        select(socketUDP+1, &fd, NULL, NULL, &timeout);
                    
                        if(FD_ISSET(socketUDP, &fd)!=0){
                            recvfrom(socketUDP, buffer_ACK, sizeof(buffer_ACK),0,(struct sockaddr *)&client_UDP, &b);
                            printf("ACK RECEIVED : %s \n", buffer_ACK);
                            char a[6];
                            memcpy(a, buffer_ACK+3, 6);
                            printf(" compare %d\n", atoi(a));
                            
                            int seq = atoi(a);
                            printf(" seq %d\n", numSequence);
                            if (seq == numSequence){
                                send = 1;                        
                            }
                    
                        }else{
                            //TIMEOUT SELECT 1 SECOND -> TO CHANGE
                            struct timeval timeout = {1, 0};
                        }
                    }

                    numSequence +=1;
                }

            //}

            EXIT_SUCCESS;
            printf("fin\n");
            printf("taille sdbuff %d\n", sizeof(tableau_seq[56].seq));
            printf("taille du buffer total : %ld\n", sizeof(buffer_data_seq[10]));
            sendto(socketUDP, (const char*)&end, sizeof(end), 0, (struct sockaddr *)&client_UDP, b);

            printf("File send correctly \n");           
            
        }        
    }
   
return (0);
}

