/* Simple udp server with stop and wait functionality */
#include<stdio.h> //printf
#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include<arpa/inet.h>
#include<sys/socket.h>
#include<pthread.h>
#include <unistd.h>
 
#define BUFLEN 100  //Max length of buffer
#define PORT 1234   //The port on which to listen for incoming data
#define DROP_RATE 10

int client_channel[2];
 
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

FILE *fp;

void *receive_pkt(void*para){

	int channelno = *(int*)para;
	DATA_PKT rcv_pkt;
    ACK_PKT  ack_pkt;
	int state =0;
	int recv_len;
	
    while(1)
    {	     
	//if(state == 0){
	//	printf("Waiting for packet 0 from sender...\n");
	//}
	//else if(state == 1){
	//	printf("Waiting for packet 1 from sender...\n");
	//}
	switch(state)
	  {  case 0:
		{   
			//printf("Waiting for packet 0 from sender...\n");
                    //fflush(stdout);
         
         //try to receive some data, this is a blocking call
         //if ((recv_len = recvfrom(s, &rcv_pkt, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1)
         if((recv_len = recv(client_channel[channelno], &rcv_pkt, sizeof(rcv_pkt),0))==0)            
		    {
                        die("recvfrom()");
                     }
	printf("RCVD PKT:OFFSET %d of size BUFLEN Bytes\n from channel %d\n\n", rcv_pkt.offset, rcv_pkt.pad+1);
		int tmp_rand = returnRandom(1,(int)(100/DROP_RATE) + 1);
		if(tmp_rand!=1){
        //if(1){
	            if (rcv_pkt.sq_no==0)
                     {  //printf("Packet received with seq. no. %d and Packet content is = %s\n",rcv_pkt.sq_no, rcv_pkt.data);
						fseek(fp, rcv_pkt.offset, SEEK_SET); 
    					fprintf(fp, "%s", rcv_pkt.data);
				      ack_pkt.sq_no = 0;
					  ack_pkt.offset = rcv_pkt.offset;
					ack_pkt.pad = rcv_pkt.pad;				
	  //if (sendto(s, &ack_pkt, recv_len, 0, (struct sockaddr*) &si_other, slen) == -1)
          if(send(client_channel[channelno], &ack_pkt, recv_len, 0) == -1)		{
           		    die("sendto()");
                  }
		printf("SENT ACK:for PKT with OFFSET %d\n from channel %d\n\n", rcv_pkt.offset, ack_pkt.pad+1);
                        state = 0;
                        break;
                  }
			}
		else{
			printf("DROP PKT\n\n");
		}
			break;
		 }
	   case 1:
                   {   //printf("Waiting for packet 1 from sender...\n");
                       //fflush(stdout);
         
                    //try to receive some data, this is a blocking call
         //if ((recv_len = recvfrom(s, &rcv_pkt, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1)
         if((recv_len = recv(client_channel[channelno], &rcv_pkt, sizeof(rcv_pkt), 0))==0)  
	   	     {
                        die("recvfrom()");
                     }
		  if (rcv_pkt.sq_no==1)
                 { printf("Packet received with seq. no.= %d and Packet content is= %s\n",rcv_pkt.sq_no, rcv_pkt.data);
			 ack_pkt.sq_no = 1;
	  // if (sendto(s, &ack_pkt, recv_len, 0, (struct sockaddr*) &si_other, slen) == -1)
          if(send(client_channel[channelno], &ack_pkt, recv_len,0)==-1)		
		       {    die("sendto()"); }
                       state = 0;
                       break;
            }
	}
	}
	printf("State %d\n",state);   
    }
}

int main(void)
{
	fp = fopen("output.txt", "w");
    int cc = 0;
    struct sockaddr_in si_me, si_other;
    int s, i, slen = sizeof(si_other);
    //char buf[BUFLEN];
    //DATA_PKT rcv_pkt;
    //ACK_PKT  ack_pkt;
    //create a TCP socket
    if ((s=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
        die("socket");
    }
    // zero out the structure
    memset((char *) &si_me, 0, sizeof(si_me));
     
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
     
    //bind socket to port
    if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
    {
        die("bind");
    }
    if (listen(s, 10) < 0){
        die("listen");
    }
    int client_socket; 
	//= accept(s, (struct sockaddr*)&si_other, &slen);
    //if(slen<0){
    //	die("socket");
    //}
	pthread_t tc[2];
    int c1 = 0;
    int c2 = 1;
	while(1) {
		if((client_socket = accept(s, (struct sockaddr*)&si_other, &slen)) < 0) {
			perror("accept unsuccessful");
			exit(1);
		}
		//pthread_mutex_lock(&mutex);
		client_channel[cc] = client_socket;
		if(cc ==0)
			pthread_create(&tc[cc],NULL,receive_pkt,&c1);
		else
			pthread_create(&tc[cc],NULL,receive_pkt,&c2);
	    cc++;
		//pthread_mutex_unlock(&mutex);
	}
	
	pthread_join(tc[0], NULL);
	pthread_join(tc[1], NULL);
    close(s);
    return 0;
}
