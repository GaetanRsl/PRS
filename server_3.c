#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <math.h>

#define RCVSIZE 1500
#define SEQSIZE 6
#define MAXPACKET 6640
#define DATASIZE 1494
#define CWND 15.0
#define TIMEOUTS 0
#define TIMEOUTUS 15000

//STRUCTURE PACKET
struct packet{
    int numSeq; //not so useful
    char Seq[DATASIZE];
    int acked;
    int size;
} ;
/*
*FONCTION file->list_packet and return 0: FICHIER PAS FINI   1: FINI
*/
int file_to_list_1Mo(FILE *src, struct packet* list_packet, int *param, int *nb_bc){
    //printf("param;%d | indice %d\n", *param, *indice);
    bzero(list_packet,(MAXPACKET)*sizeof(struct packet));
    char buff_read[DATASIZE];
    bzero(buff_read,DATASIZE);
    int indice =1;
    int res_read=0;
    int fini=0;
    int total=0;
    //printf("entered fonction file to list\n");
    //printf("indice = %d\n",indice);
    
    while(indice<=MAXPACKET && (res_read=fread(buff_read,sizeof(char),DATASIZE,src) ) > 0){

        //printf("entered while indice < MAXPCKT\n");
        
        memcpy(list_packet[indice].Seq, buff_read, res_read);
        list_packet[indice].numSeq=indice+*param;
        list_packet[indice].acked=0;
        list_packet[indice].size= res_read;
        total=total+res_read;
        
        indice=indice+1;
    }

    printf("TOTAL BYTES IN TABLE : %d\n ",total);
    *param=*param+indice-1;
    
    if(indice<MAXPACKET){
        fini=1;
        printf("finished reading the file FINI==1\n");
    }
    *nb_bc+=1;
    return(fini);
}
/*
*  FONCTION log (windowsize, mode, ssthresh, file, time)
*/
/*
void logtofile(double cwnd,int mode, int ssthresh, FILE* log,struct timeval t){
    char *virgul = ",";
    //buffer size
    char cwnd_buff[318];
    sprintf(cwnd_buff, "%d", (int)cwnd);
    fputs(cwnd_buff, log);
    fputs(virgul, log);
    
    //mode
    char mode_buff[318];
    sprintf(mode_buff, "%d", mode);
    fputs(mode_buff, log);
    fputs(virgul, log);

    //ssthresh
    char ssth_buff[318];
    sprintf(ssth_buff, "%d", ssthresh);
    fputs(ssth_buff, log);
    fputs(virgul, log);

    //time
    struct timeval a;
    gettimeofday(&a, 0);
    struct timeval diff;
    diff.tv_sec = (a.tv_sec - t.tv_sec);
    diff.tv_usec = (a.tv_usec - t.tv_usec);

    long b = diff.tv_sec*1000000+diff.tv_usec;
    
    sprintf(mode_buff, "%ld", b);
    fputs(mode_buff, log);

    //end if line
    char *virgule = ",\n";
    fputs(virgule, log);
} */

/*
*FONCTION puts Sequence number + Data in a buffer
*/
void packetToBuff(struct packet p1, char buffer[RCVSIZE] ){
    bzero(buffer,RCVSIZE);
    int nbSeq = p1.numSeq;
    sprintf(buffer, "%6d", nbSeq);
    memcpy(buffer+6, p1.Seq , p1.size);
}

int max(int a,int b){
    if(a>b){
        return(a);
    }else{
        return(b);
    }
}

int main(int argc, char* argv[]){
    if (argc !=4){
        perror("wrong nb of arguments: ./server portUDP \n");
        exit(EXIT_FAILURE);
    }
    int windowsize = atoi(argv[2]);
    int timeout_us = atoi(argv[3]);

    /*
    *CREATION OF SOCKET HANDSHAKE
    */
    //PORT
    int nb_port_acc = atoi(argv[1]);
    //ADDR
    struct sockaddr_in addr_hsk;
    struct sockaddr_in client;
    memset((char*)&addr_hsk, 0, sizeof(addr_hsk));
    memset((char*)&client, 0, sizeof(client));
    socklen_t socklen = sizeof(addr_hsk);
    //socklen_t socklen2 =sizeof(client);
    addr_hsk.sin_addr.s_addr=INADDR_ANY;
    addr_hsk.sin_family=AF_INET;
    addr_hsk.sin_port = htons(nb_port_acc);
    //SOCKET
    int sock_hsk=socket(AF_INET, SOCK_DGRAM, 0);
    if(sock_hsk<0){
        perror("socket UDP creation failed");
        exit(EXIT_FAILURE);
    };
    int fd_max = (sock_hsk+1);
    //REUSE
    int reuse =1;
    setsockopt(sock_hsk, SOL_SOCKET,SO_REUSEADDR, &reuse, sizeof(reuse));
    //BIND
    int res_bind_udp = bind(sock_hsk, (struct sockaddr*)&addr_hsk, sizeof(addr_hsk));
    if(res_bind_udp<0){
        perror("binding 2 failed");
        exit(EXIT_FAILURE);  
    };

    //BUFFER
    char buff_acc[RCVSIZE];
    char buff_com[RCVSIZE];
    char buff_ack_2[SEQSIZE+3]; 
    char buff_ack_seq[SEQSIZE];  
    char buff_send[RCVSIZE];

    //messages
    char msg_syn_ack[12] ="SYN-ACK";
    char msg_fin[4]="FIN";

    /*
    *FILE for log CWND and SSTHRESH
    */
    /*
    FILE *file =fopen("log.txt","w");
    char file_buff[318];
    sprintf(file_buff, "timeout(ms) : %d \n", TIMEOUTUS);
    fputs(file_buff, file);
    bzero(file_buff,318);
    sprintf(file_buff, "window init : %f \n", INITCWND);
    fputs(file_buff, file);
    bzero(file_buff,318);
    sprintf(file_buff,"max window : %f \n",MAXCWND);
    fputs(file_buff, file);
    */

    //SELECT
    fd_set fd_hsk;
    FD_ZERO(&fd_hsk);
    fd_set fd_ack;
    FD_ZERO(&fd_ack);
    struct timeval timeout={TIMEOUTS,timeout_us};

    //global values
    int nb_packets;
    pid_t pid_pere = getpid(); //recupere pid pere

    //while(1){
        //printf("Beginning of while: \n");

        //TEST si processuss == pere
        pid_t pid_now = getpid();
        if(pid_now==pid_pere){
            FD_SET(sock_hsk, &fd_hsk); 
        }
        //SELECT
        select(fd_max,&fd_hsk,NULL,NULL,NULL);
        //printf("Select hsk deblocked \n");
 
        //HANDSHAKE
        if(FD_ISSET(sock_hsk,&fd_hsk)){
            //SYN
            memset(buff_acc, 0, RCVSIZE);
            int res_recv= recvfrom(sock_hsk, &buff_acc, RCVSIZE, 0, (struct sockaddr*)&client, &socklen);
            if(res_recv<=0){
                perror("recv_udp failed\n"); 
                exit(EXIT_FAILURE); 
            }
            if(strncmp(buff_acc,"SYN",3) == 0){
                printf("Client > %s \n", buff_acc);
            } else {
                perror("error while receiving SYN\n"); 
                exit(EXIT_FAILURE); 
            }   
            /*
            *CREATION OF SECOND SOCKET COMMUNICATION
            */
            //PORT "DYNAMIQUE"
            int port_priv=(rand()%8999)+1000;

            //ADDR
            struct sockaddr_in addr_com;
            memset((char*)&addr_com, 0, sizeof(addr_com));
            addr_com.sin_addr.s_addr=INADDR_ANY;
            addr_com.sin_family=AF_INET;
            addr_com.sin_port = htons(port_priv);

            //SOCKET
            int sock_com=socket(AF_INET, SOCK_DGRAM, 0);
            if(sock_com<0){
                perror("socket COM creation failed");
                exit(EXIT_FAILURE);
            };
            int fd_max_ack = (sock_com+1);
            //REUSE
            setsockopt(sock_com, SOL_SOCKET,SO_REUSEADDR, &reuse, sizeof(reuse));
            //BIND
            int res_bind_com = bind(sock_com, (struct sockaddr*)&addr_com, sizeof(addr_com));
            if(res_bind_com<0){
                perror("binding failed");
                exit(EXIT_FAILURE);  
            }
            
            //SYN-ACK
            sprintf(msg_syn_ack+7,"%d",port_priv);
            int hsk_syn_ack = sendto(sock_hsk, (const char*)&msg_syn_ack, sizeof(msg_syn_ack),0, (struct sockaddr *)&client, socklen);
            if(hsk_syn_ack<=0){
                perror("send hsk_syn_ack failed\n"); 
                exit(EXIT_FAILURE); 
            } 
            printf("Server > %s \n",msg_syn_ack);
            
            //FORK
            int pid=fork();
            //printf("fork created -> %d\n",pid);
            
            if (pid >0){ //PERE
                //ACK
                int hsk_ack= recvfrom(sock_hsk, &buff_acc, RCVSIZE, 0, (struct sockaddr*)&client, &socklen);
                if(hsk_ack<=0){
                    perror("recv hsk_ack failed\n"); 
                    exit(EXIT_FAILURE); 
                }
                if(strncmp(buff_acc,"ACK",3)==0){
                    printf("Client > %s\n",buff_acc);
                    printf("Connection etablie! \n");
                } else {
                    perror("error while receiving ACK\n"); 
                    exit(EXIT_FAILURE); 
                }
                close(sock_com);

            } else if(pid==0){ //FILS
                close(sock_hsk);

                /*
                *TRANSFERT FILE
                */
                //printf("Entered transfer mode: \n");
                bzero(buff_com,RCVSIZE);
                
                //RECEIVE FILE NAME
                int res_recv= recvfrom(sock_com, &buff_com, RCVSIZE, 0, (struct sockaddr*)&client, &socklen);
                if(res_recv<=0){
                    perror("recv_com failed"); 
                    exit(EXIT_FAILURE); 
                }
                //OPEN THE FILE
                FILE *src =fopen(buff_com,"r");
                if(src==NULL){
                    perror("could not open the file \n");
                    return(-1);
                }
                
                //CREATE PACKET LIST 
                struct packet *list_packet=malloc(sizeof(struct packet) * MAXPACKET+1);

                //parameters for function file_to_list_1Mo
                int param=0; //nb de packets mis dans le tableau
                int fini=0;
                int nb_boucle=0;
                
                //LOOP SEND EACH PACKET 
                int nb_ack=0;
                int fake_ack=0; //donne indice dans le tableau de paquet correspondant

                int last_acked=0;
                int last_sent=0;
                
                /*
                struct timeval t;
                gettimeofday(&t, 0);
                */
                int case_doublon = 0; //boolean when entered doublon state
                int doublon_nb = 0; //nb sequence du paquet en doublon

                while(fini==0){
                
                    fini=file_to_list_1Mo(src,list_packet,&param,&nb_boucle); 
                    //printf("fini: %d \n ",fini);
                    printf("param apres fonction : %d\n",param);
                    nb_packets=param;
                    //printf("Nb of packets to send: %d \n", nb_packets);
                    
                    int start=1;
                    //double windowsize = CWND;
                    int end=(start+windowsize)-1; 
                    //printf("BEFORE WHILE last_acked=%d, nb_packets=%d \n",last_acked,nb_packets);
                    
                    while(last_acked<nb_packets){ 
                        printf("revenu dans le while last_acked<nb_packets\n");
                        //logtofile(windowsize, mode, ssthresh,file,t);
                        
                        //WINDOW
                        //printf("START %d , END %d \n",start,end);

                        if(fake_ack>0){ //ack corresponding to group before -> do nothing
                            
                            for(int i=start; i<=end; i++){
                                //printf("entered for start%d ->end%d \n",start,end);
                                bzero(buff_send,RCVSIZE);
                                packetToBuff(list_packet[i], buff_send);

                                //printf("before if inside for numseq=%d, last_sent=%d\n",list_packet[i].numSeq,last_sent);
                                if(list_packet[i].numSeq > last_sent){
                                    int res_sent = sendto(sock_com, buff_send, list_packet[i].size+6,0, (struct sockaddr *)&client, socklen);
                                    //printf("Sending packet %d, nb seq %d, (%d bytes) \n",i,list_packet[i].numSeq,res_sent);
                                    if(res_sent<=0){
                                        perror("res_sent failed"); 
                                        exit(EXIT_FAILURE); 
                                    }
                            
                                    last_sent=list_packet[i].numSeq;
                                }
                            } //FIN window
                        }
                        printf("end of for start->end\n");

                        //SELECT WITH TIMEOUT
                        FD_SET(sock_com,&fd_ack);
                        select(fd_max_ack,&fd_ack,NULL,NULL,&timeout); 
                        printf("after select\n");

                        //ACK RECEIVED
                        if(FD_ISSET(sock_com,&fd_ack)){

                            //printf("select deblocked -> ack received \n");
                            bzero(buff_ack_2,SEQSIZE+3);
                            int res_ack = recvfrom(sock_com, &buff_ack_2, sizeof(buff_ack_2), 0, (struct sockaddr*)&client, &socklen);
                            if(res_ack<=0){
                                perror("res_ack failed\n"); 
                                exit(EXIT_FAILURE); 
                            }
                            
                            memcpy(buff_ack_seq,buff_ack_2+3,SEQSIZE);
                            nb_ack=atoi(buff_ack_seq);   
                            printf("ACK received: %d\n",nb_ack);
                            fake_ack=nb_ack-((MAXPACKET)*(nb_boucle-1));
                            printf("fake ack:%d\n",fake_ack);
                            printf("last acked: %d\n",last_acked);

                            if(fake_ack>0){
                                list_packet[fake_ack].acked+=1;
                                
                                //treatement ACK
                                if(nb_ack>last_acked){ 
                                    last_acked=nb_ack;
                                    //fake_last_acked=last_acked -((MAXPACKET)*(nb_boucle-1));
                                    
                                    //slide window
                                    start=fake_ack+1;
                                    if((nb_ack+windowsize)<nb_packets){
                                        end=fake_ack+windowsize;
                                    }else{
                                        end=nb_packets-((MAXPACKET)*(nb_boucle-1));
                                    }
                                    printf("last_acked=%d | start=%d | end=%d\n",last_acked,start,end);
                                } else {     
                                    //DOUBLON ACK
                                    if(list_packet[fake_ack].acked>2){
                                        if(list_packet[fake_ack].acked==3){
                                            case_doublon=1;
                                            doublon_nb=nb_ack;

                                        }//else if(list_packet[nb_ack].acked>3){
                                            //printf("same doublon - window size= %f\n",windowsize);
                                        //}

                                        if(case_doublon){

                                            bzero(buff_send,RCVSIZE);
                                            packetToBuff(list_packet[fake_ack+1], buff_send);
                                            int res_sent_db = sendto(sock_com, buff_send, list_packet[fake_ack+1].size+6,0, (struct sockaddr *)&client, socklen);
                                            printf("doublon, resending: %d \n",fake_ack+1);
                                            if(res_sent_db<=0){
                                                perror("res_sent_db failed"); 
                                                exit(EXIT_FAILURE); 
                                            }
                                            //printf("DOUBLON ! Error while sending: %d, resending! \n",nb_ack+1);
                                            printf("fake_ack=%d\n",fake_ack);
                                            case_doublon=0;
                                        }
                                        //printf("Doublon count : %d, of paquet %d\n",list_packet[fake_ack].acked,nb_ack);
                                        
                                    }else{
                                        //le premier et second doublon
                                    }
                                }//FIN traitement nb_ack

                                if(nb_ack==doublon_nb+1){
                                    //printf("DOUBLON ACK RECEIVED: %d\n",nb_ack);
                                    case_doublon=0;
                                    doublon_nb=0;
                                }
                            }else{
                                printf("IN THE ELSE\n");
                                timeout.tv_sec=TIMEOUTS;
                                timeout.tv_usec=timeout_us;
                                //break;
                            }
                            printf("finished if fake_ack>0 \n");

                        } else { //fin FDISSET
                            //TIMEOUT
                            //printf("TIMEOUT - windowSize= %f | SSthresh=%d \n",windowsize,ssthresh);

                            bzero(buff_send,RCVSIZE);
                            packetToBuff(list_packet[fake_ack+1], buff_send);
                            int res_sent_err = sendto(sock_com, buff_send,  list_packet[fake_ack+1].size+6 ,0, (struct sockaddr *)&client, socklen);
                            printf("last ack: %d -> fake: %d \n",nb_ack,fake_ack);
                            printf("TIMEOUT: SENDING %d \n",fake_ack+1);
                            if(res_sent_err<=0){
                                perror("res_sent_err failed"); 
                                exit(EXIT_FAILURE); 
                            }
                           
                            //reset timeout
                            timeout.tv_sec=TIMEOUTS;
                            timeout.tv_usec=timeout_us;

                        } // FIN else ifset(sock_com)
                        printf("finished fd isset\n");

                    } // FIN loop
                    printf("fini main loop\n");
                   
                }//FIN loop while(fini==0);
                
                /*
                *FIN
                */
                //printf("before FIN, last_acked=%d, nb_packets=%d, fini=%d\n",last_acked,nb_packets,fini);
                if(last_acked==nb_packets && fini){
                    int res_fin = sendto(sock_com, (const char*)&msg_fin, strlen(msg_fin),0, (struct sockaddr *)&client, socklen);
                    if(res_fin<=0){
                        perror("res_fin failed"); 
                        exit(EXIT_FAILURE); 
                    }
                    printf("sent FIN\n");
                }

                fclose(src);
                close(sock_com);
                free(list_packet);
                exit(EXIT_SUCCESS);
            
            } //FIN pid==0/boucle de transfert

           
        } //FIN  IFSET(sock_hsk)   
       
    //} //FIN WHILE (1)
    
    return(0);
} //FIN MAIN