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
    int acked;
};

int main(int argc, char *argv[]) {

    struct timeval timeout = {1, 0};

    /*
    * Creation socket Handshake
    * */
    
    struct sockaddr_in client_UDPConnection;
    struct sockaddr_in address_UDPConnection;
    memset((char*)&address_UDPConnection, 0, sizeof(address_UDPConnection));
    int socketUDP_connection = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketUDP_connection<0){
        printf("pb sck udp connect\n");
        return -1;
    }

    address_UDPConnection.sin_addr.s_addr = htonl(INADDR_ANY);
    address_UDPConnection.sin_family = AF_INET;
    address_UDPConnection.sin_port=htons(atoi(argv[1]));

    int bind2 = bind(socketUDP_connection, (struct sockaddr *)&address_UDPConnection, sizeof(address_UDPConnection));
    if (bind2<0){
        printf("problem socket UDP Connection\n");
        return -1;
    }

    struct sockaddr init_udpConection;
    socklen_t v = sizeof(init_udpConection); 
    
    /*
    * Declaration des buffers
    * */

    char buffer_UDP[15];
    char buffer_ACK[10];
    char buffer_seq[7];
    char buffer_data_seq[1035];
    char msg_SYNACK[]="SYN-ACK";
    char buffer_UDPConnection[10];
    char fs_name[15];
    char FIN[4]="FIN";

    fd_set fd;
    FD_ZERO(&fd);

    fd_set fd_conn;
    FD_ZERO(&fd_conn);

    if (argc != 2){
        printf("Nombre d'arguments invalide \n");
        printf("syntaxe : ./serveur <port serveur UDP Connection>\n");
        exit(EXIT_FAILURE);
    }

    /*
    * Boucle pricipal qui gere les multiples clients
    * */

    struct elem* tableau_seq=malloc(sizeof(struct elem )* 10000);

    pid_t pid_pere = getpid();
    while(1){
        printf("Waiting for messages... \n");

        pid_t pid_actuel = getpid();
        if (pid_actuel == pid_pere){
            FD_SET(socketUDP_connection, &fd);
        }

        int slct = select(socketUDP_connection+1, &fd, NULL, NULL, NULL);
        
        if(slct <0){
            printf("Error Select \n");
            return -1;
        }
       
        /*
        * 3 Way Handshake 
        * */

        if(FD_ISSET(socketUDP_connection, &fd)){

            printf("Beginning of the connection\n");
            //SYN RCV
            recvfrom(socketUDP_connection, &buffer_UDPConnection, sizeof(buffer_UDPConnection),0, (struct sockaddr *)&client_UDPConnection, &v);
            printf("First message from UDP Client : %s\n", buffer_UDPConnection);

            /*
            * Creation socket transfert fichier
            * */
            
            //int port_prive = 3456;
            //char private_port[]="3456";
            struct sockaddr_in address_UDP;
            struct sockaddr_in client_UDP;
            memset((char*)&address_UDP, 0, sizeof(address_UDP));
            int socketUDP = socket(AF_INET, SOCK_DGRAM, 0);
            printf("Socket UDP cree\n");  
            if (socketUDP <0){
                printf("pb socket UDP\n");
                return -1;
            }
            int valid = 1;
            setsockopt(socketUDP, SOL_SOCKET, SO_REUSEADDR, &valid, sizeof(int));
            int port_prive = (rand()%8999)+1000;
            sprintf(msg_SYNACK, '%d', port_prive);
            address_UDP.sin_addr.s_addr = htonl(INADDR_ANY);
            address_UDP.sin_family = AF_INET;
            address_UDP.sin_port=htons(port_prive);

            int t = bind(socketUDP, (struct sockaddr *)&address_UDP, sizeof(address_UDP));
            if (t<0){
                printf("problem socket UDP\n");
                return -1;
            }
            struct sockaddr init_udp;
            socklen_t b = sizeof(init_udp);
            
            if(strncmp(buffer_UDPConnection, "SYN",3) ==0){
                //SYN-ACK SEND
                //strcat(msg_SYNACK, private_port);
                
                sendto(socketUDP_connection, (const char*)&msg_SYNACK, sizeof(msg_SYNACK),0, (struct sockaddr *)&client_UDPConnection, v);           
            }else{
                exit(EXIT_FAILURE);
            }
            int pid = fork();
            if (pid > 0){
                //ACK RCV
                recvfrom(socketUDP_connection, &buffer_UDPConnection, sizeof(buffer_UDPConnection),0, (struct sockaddr *)&client_UDPConnection, &v);
                printf("Second message from client : %s \n", buffer_UDPConnection);
                if(strncmp(buffer_UDPConnection, "ACK",3) ==0){
                    printf("Succesful 3 way handshake\n");           
                }else{
                    exit(EXIT_FAILURE);
                }
               
                close(socketUDP);

            } else if(pid == 0){
                close(socketUDP_connection);
                printf("ENTER FILE TRANSFERT\n");    

                //On recoit le nom du fichier a envoye   

                int recv = recvfrom(socketUDP, buffer_UDP, sizeof(buffer_UDP),0, (struct sockaddr *)&client_UDP, &b);
                int file_sz = sizeof(buffer_UDP);
                memcpy(fs_name, buffer_UDP, file_sz);
                char sdbuf[1024];
                FILE *fs = fopen(fs_name, "r");

                printf("port : %d\n", ntohs(client_UDP.sin_port));
                printf("Message client UDP : %s\n", buffer_UDP);
                sendto(socketUDP, buffer_UDP, recv,0, (struct sockaddr *)&client_UDP, b);
                
                if (fs == NULL){
                    printf("Error, file %s not found \n", fs_name);
                    exit(1);
                }
                int fs_block_sz = 0;
                bzero(sdbuf, 1024);
                int nbPaquet = 1;

                /*
                * Boucle qui lit le fichier en entier 
                **/

                while((fs_block_sz = fread(sdbuf, sizeof(char), 1024,fs)) > 0 ){
                    //printf("avant\n");
                    memcpy(tableau_seq[nbPaquet].seq, sdbuf,1024);
                    tableau_seq[nbPaquet].acked = 0;
                    //printf("apres\n");
                    nbPaquet+=1;
                }
                printf("NOMBRE PAQUET : %d\n", nbPaquet);
        
                int last_sent = 0;
                int seq = 0;
                int last_ack=0;
                int start_window=1;
                int end_window = 5;

                /*
                * Boucle d'envoie tant que tous les paquets ne sont pas ACK
                * */

                while(last_ack < nbPaquet){

                    for(int i = start_window; i<=end_window; i++){
                        //Reinitialisation des buffers
                        bzero(buffer_seq, 6);
                        bzero(buffer_data_seq, 1030);

                        //Ajout data + seq dans buffer
                        sprintf(buffer_seq,"%6d", i);
                        memcpy(buffer_data_seq, buffer_seq, 6);
                        memcpy(buffer_data_seq + 6,tableau_seq[i].seq, 1024);
                        
                        if(i>last_sent){
                            sendto(socketUDP, buffer_data_seq, 1030, 0, (struct sockaddr *)&client_UDP, b);
                            printf("Paquet envoye : %d\n", i);
                            last_sent = i;
                        }                                       
                    }

                    FD_SET(socketUDP, &fd_conn);
                    select(socketUDP+1, &fd_conn, NULL, NULL, &timeout);
                        
                    if(FD_ISSET(socketUDP, &fd_conn)!=0){
                        
                        recvfrom(socketUDP, buffer_ACK, sizeof(buffer_ACK),0,(struct sockaddr *)&client_UDP, &b);
                        printf("ACK RECEIVED : %s \n", buffer_ACK);
                        
                        char a[6];
                        memcpy(a, buffer_ACK+3, 6);                   
                                
                        seq = atoi(a);
                        tableau_seq[seq].acked +=1;

                        if (seq > last_ack){
                            last_ack = seq;
                            start_window = seq +1;
                            if ((seq + 5)< nbPaquet){
                                end_window = seq + 5;
                            }else{
                                end_window=nbPaquet;
                            }
                        }else{
                            if(tableau_seq[seq].acked > 1){
                            printf("Sequence %d mal recu !!\n", seq+1);

                            sprintf(buffer_seq,"%6d", seq + 1);
                            memcpy(buffer_data_seq, buffer_seq, 6);
                            memcpy(buffer_data_seq + 6,tableau_seq[seq+1].seq, 1024);

                            sendto(socketUDP, buffer_data_seq, 1030, 0, (struct sockaddr *)&client_UDP, b);
                            tableau_seq[seq].acked = 1;
                            }
                        }         
                                       
                    }else{

                        /*
                        * When the ACK is not received, we send again last ACK + 1 packet
                        **/    

                        sprintf(buffer_seq,"%6d", last_ack + 1);
                        memcpy(buffer_data_seq, buffer_seq, 6);
                        memcpy(buffer_data_seq + 6,tableau_seq[last_ack+1].seq, 1024);
                        sendto(socketUDP, buffer_data_seq, 1030, 0, (struct sockaddr *)&client_UDP, b);
                        printf("Paquet %d envoye car ACK non recu\n", last_ack+1);
                

                        timeout.tv_sec = 1;
                        timeout.tv_usec=0;           
                    }
                } //crochet fin while last ack < nb packet
                /*
                * Sending end of file message and close connection
                * */

                sendto(socketUDP, (const char*)&FIN, sizeof(FIN), 0, (struct sockaddr *)&client_UDP, b);
                printf("FIN \n");

                printf("File send correctly \n");           
                free(tableau_seq);
                close(socketUDP);
                exit(EXIT_SUCCESS);
                        
            }           
            
        }
        //else if(FD_ISSET(socketUDP, &fd)){
           
        //} crochet fin FD ISSET #1


    } //crochet fin while(1)
    close(socketUDP_connection);   
    return (0);
}