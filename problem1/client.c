/*
    Simple udp client with stop and wait functionality
*/
#include<stdio.h> //printf
#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include<arpa/inet.h>
#include<sys/socket.h>
#include<pthread.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>  
 
#define BUFLEN 100  //Max length of buffer
#define PORT 1234   //The port on which to send data
#define DROP_RATE 10

typedef struct para_route{
	int soc;
	int channel_no;
}para_struc;

typedef struct packet1{
    int sq_no;
    int offset;
	int pad;
}ACK_PKT;

typedef struct packet2{
    int sq_no;
    int offset;
	int pad;
    char data[BUFLEN];
}DATA_PKT;

void die(char *s)
{
    perror(s);
    exit(1);
}
 
int returnRandom(int lower, int upper) 
{ 
	int num = (abs(rand()) % (upper - lower + 1)) + lower; 
	return num;	
}

DATA_PKT rcv1_ack,rcv0_ack;
bool channel0_rcv = false;
bool channel1_rcv = false;
FILE *fp;
int main_offset = 0;
int which_channel = 0;

//char buf[BUFLEN];
//char message[BUFLEN];
//DATA_PKT send_pkt,rcv_ack;


void *receive_rcv(void* struc_para){
	para_struc *tmp_struc = (para_struc*)struc_para;

while(1){
	if(tmp_struc->channel_no == 0){
		if(recv(tmp_struc->soc ,&rcv0_ack, sizeof(rcv0_ack), 0)==-1)
				{
            			  die("recvfrom()");
   	     		}
		channel0_rcv = true;
	}
	else{
		if(recv(tmp_struc->soc ,&rcv0_ack, sizeof(rcv0_ack), 0)==-1)
				{
   	         		  die("recvfrom()");
	        	}
		channel1_rcv = true;
	}
		
}
}


void *channelCreate(void *para){
    
struct sockaddr_in si_other;
int s, i, slen=sizeof(si_other), s1;
char buf[BUFLEN];
char message[BUFLEN];
DATA_PKT send_pkt,rcv_ack;
int channel = *(int*)para;
pthread_t rcv1;

if ( (s=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
        die("socket");
    }
 
    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);
    //if()
    si_other.sin_addr.s_addr = inet_addr("127.0.0.1");

    int c = connect(s, (struct sockaddr*)&si_other, sizeof(si_other));
    if(c<0){
    	die("establish connection");
    }
para_struc tmp;
tmp.soc = s;
tmp.channel_no = channel;
pthread_create(&rcv1, NULL,receive_rcv, &tmp);
 int state = 0;
    while(1)
    {      

	switch(state)
	{  case 0: //printf("Enter message %d channel: ", channel);//wait for sending packet with seq. no. 0
                  //fgets(send_pkt.data,sizeof(send_pkt),stdin);
 		  send_pkt.sq_no = 0;
			memset(send_pkt.data, '\0', sizeof(send_pkt.data));
		  //if (sendto(s, &send_pkt, sizeof(send_pkt), 0 , (struct sockaddr *) &si_other, slen)==-1)
			fseek(fp, main_offset, SEEK_SET); 
    		fread(send_pkt.data, sizeof(char), BUFLEN, fp);
			send_pkt.offset = main_offset;
		if(send_pkt.data[0] == '\0'){
				exit(1);
			}
//		int tmp_rand = returnRandom(1,(int)(100/DROP_RATE) + 1);
//		printf("%d\n",tmp_rand);
		if(which_channel == 0){
			if(1){
			send_pkt.pad = which_channel;
//			if(tmp_rand!=1){
        	  if(send(s, &send_pkt, sizeof(send_pkt), 0)==-1)	
		  		{
            	  die("sendto()");
        		}
			printf("SENT PKT: OFFSET %d of size %d Bytes\nfrom channel %d\n\n", send_pkt.offset, BUFLEN, which_channel+1);
				//printf("0\n");
			}
		  //which_channel = 1;
			state = 1;
			break;
		}
		if(which_channel == 1){
			if(1){
			send_pkt.pad = which_channel;
//			if(tmp_rand!=1){
	        	  if(send(s, &send_pkt, sizeof(send_pkt), 0)==-1)	
			  	{
	            	  die("sendto()");
	        		}
				printf("SENT PKT: OFFSET %d of size %d Bytes\nfrom channel %d\n\n", send_pkt.offset, BUFLEN, which_channel+1);
				//printf("1\n");
			}
			//which_channel = 0;
			state = 1;
			break;
		}
			
			state = 1;
		break;
			
				 
		       
	   case 1:   //waiting for ACK 0
                  //if (recvfrom(s, &rcv_ack, sizeof(rcv_ack), 0, (struct sockaddr *) &si_other, &slen) == -1)
        	//if(recv(s,&rcv_ack, sizeof(rcv_ack), 0)==-1)
			//{
            		  //die("recvfrom()");
        		//}
		
		if(channel ==0){
			if(channel0_rcv == false){
				clock_t t = clock();
				double time_t;
				while(channel0_rcv == false){
					time_t = (double)(clock()-t)/CLOCKS_PER_SEC;
					//printf("%lf",time_t);
					if(time_t>2.00){
						break;
					}
				}
				if(channel0_rcv == false){
				printf("TIMEOUT channel %d\n\n", which_channel+1);					
				which_channel = 1-which_channel;				
					state = 0;
					//printf("chnage\n");
					break;
				}
				channel0_rcv = false;
				//printf("RCVD PKT: OFFSET");
			}
			  if (rcv0_ack.sq_no==0 && rcv0_ack.offset == main_offset)
				//if (rcv0_ack.sq_no==0)
                     {  //printf("Received ack seq. no. %d\n",rcv0_ack.offset);
			printf("RCVD ACK: for PKT with OFFSET %d\n from channel %d\n\n", rcv0_ack.offset, rcv0_ack.pad+1);
							main_offset +=BUFLEN;
                             state = 0;
			     break;
                     }
				channel0_rcv = false;
		}
		else{
			if(channel1_rcv == false){
				clock_t t = clock();
				double time_t;
				while(channel1_rcv == false){
					time_t = (double)(t-clock())/CLOCKS_PER_SEC;
					if(time_t>2.00){
						break;
					}
				}
				if(channel1_rcv == false){
					which_channel = 0;
					state = 0;
					break;
				}
				channel1_rcv = false;
			}
			if (rcv1_ack.sq_no==0 && rcv1_ack.offset == main_offset)
			//if (rcv1_ack.sq_no==0)
                     {  //printf("Received ack seq. no. %d\n",rcv1_ack.offset);
			printf("RCVD ACK: for PKT with OFFSET %d\n from channel %d\n\n", rcv1_ack.offset, rcv1_ack.pad+1);
							main_offset +=BUFLEN;
                             state = 0;
			     break;
                     }
		}
		break;
		 
	    case 2:   
			 printf("Enter message 1: ");  
                  //wait for sending packet with seq. no. 1
                   fgets(send_pkt.data,sizeof(send_pkt),stdin);
 			 send_pkt.sq_no = 1;		
			//if (sendto(s, &send_pkt, sizeof(send_pkt) , 0 , (struct sockaddr *) &si_other, slen)==-1)
        	if(send(s,&send_pkt,sizeof(send_pkt),0)==-1)	
			 {
            	  die("sendto()");
        		}
			state = 3; 
			break;

                 
         case 3:	//waiting for ACK 1
		      //if(recvfrom(s, &rcv_ack, sizeof(rcv_ack), 0, (struct sockaddr *) &si_other, &slen) == -1)
        	//if(recv(s,&rcv_ack, sizeof(rcv_ack),0)==-1)	 
			//{
            //	   die("recvfrom()");
        	//	 }
			//if (rcv_ack.sq_no==1)
            //         { printf("Received ack seq. no. %d\n",rcv_ack.sq_no);
             //          state = 0; 
			 //    break;
             //        } 
		if(channel ==0){
			if(channel0_rcv == false){
				clock_t t = clock();
				double time_t;
				while(channel0_rcv == false){
					time_t = (double)(t-clock())/CLOCKS_PER_SEC;
					if(time_t>2.00){
						break;
					}
				}
				if(channel0_rcv == false){
					state = 2;
					break;
				}
				channel0_rcv = false;
			}
		  if (rcv0_ack.sq_no==1)
                     {  printf("Received ack seq. no. %d\n",rcv0_ack.sq_no);
                             state = 0;
			     break;
                     }
		}
		else{
			if(channel1_rcv == false){
				clock_t t = clock();
				double time_t;
				while(channel1_rcv == false){
					time_t = (double)(t-clock())/CLOCKS_PER_SEC;
					if(time_t>2.00){
						break;
					}
				}
				if(channel1_rcv == false){
					state = 2;
					break;
				}
				channel1_rcv = false;
			}
			if (rcv1_ack.sq_no==1)
                     {  printf("Received ack seq. no. %d\n",rcv1_ack.sq_no);
                             state = 0;
			     break;
                     }
		}      
      }

 }
	pthread_join(rcv1,NULL);
    close(s);
}



int main(void)
{
    fp = fopen("input.txt", "r");
	
    
    
    pthread_t channel12;
    int route0 = 0;
	int route1 = 1;
    pthread_create(&channel12, NULL, channelCreate, &route0);
    pthread_join(channel12, NULL);
	fclose(fp);

    return 0;
}

