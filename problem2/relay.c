/*  Simple udp server */

#include<stdio.h> //printf
#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include<arpa/inet.h>
#include<sys/socket.h>
#include<pthread.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>

#define BUFLEN 100  //Max length of buffer
#define PORT1 8881
#define PORT2 8882   //The port on which to listen for incoming data
#define PORT_S 8880
#define DROP_RATE 10


typedef struct packet1{
    //int sq_no;
    int offset;
	int pad;
}ACK_PKT;

typedef struct packet2{
    //int sq_no;
    int offset;
	int pad;
    char data[BUFLEN];
}DATA_PKT;

typedef struct send_para{
	//int s;
	int temp_s;
	struct sockaddr_in server;
	int len;
	int relayno;
	char data_para[BUFLEN];
}para_send;

char* get_current_time(){
	char* str = (char*) malloc(sizeof(char)*20);
	int rc;
	time_t curr;
	struct tm* timeptr;
	struct timeval tv;
	
	curr = time(NULL);
	timeptr = localtime(&curr);
	gettimeofday(&tv, NULL);
	rc = strftime(str, 20, "%H:%M:%S", timeptr);
	char ms[8];
	sprintf(ms, ".%06ld", tv.tv_usec);
	strcat(str, ms);
	return str;
}

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

void *relay_sock(void* para){

	int relayno = *(int*)para;
	
	struct sockaddr_in si_me, si_other, server_address;
    int s, i, slen = sizeof(si_other) , recv_len, server_len = sizeof(server_address);
    int temp_s;
    char buf[BUFLEN];
     
    //create a UDP socket
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
    if ((temp_s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
     
    // zero out the structure
    memset((char *) &si_me, 0, sizeof(si_me));
     
    si_me.sin_family = AF_INET;
	if(relayno == 0){
	    si_me.sin_port = htons(PORT1);
	}
	else{
		si_me.sin_port = htons(PORT2);
	}    
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    memset((char *) &server_address, 0, sizeof(server_address));
     
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT_S);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.3");
     
    //bind socket to port
    if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
    {
        die("bind");
    }
     
    //keep listening for data
    while(1)
    {
	DATA_PKT buff;
	ACK_PKT buff1;
        //printf("Waiting for data...");
        //fflush(stdout);
         
        //try to receive some data, this is a blocking call
        if ((recv_len = recvfrom(s, &buff, sizeof(DATA_PKT), 0, (struct sockaddr *) &si_other, &slen)) == -1)
        {
            die("recvfrom()");
        }
         
        //print details of the client/peer and the data received
        //printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
        //printf("Data: %d  %s\n" , buff.offset, buff.data);
		buff.pad = relayno;
        printf("RELAY%d\tR\t%s\tDATA\t%d\tCLIENT\tRELAY%d\n\n", relayno, get_current_time(), buff.offset,buff.pad+1); 
        //now reply the client with the same data
int tmp_rand = returnRandom(1,(int)(100/DROP_RATE) + 1);
if(tmp_rand != 1){
        if (sendto(temp_s, &buff, sizeof(DATA_PKT), 0, (struct sockaddr*) &server_address, server_len) == 0)
        {
            die("sendto()");
        }
		printf("RELAY%d\tS\t%s\tDATA\t%d\tRELAY%d\tSERVER\n\n", relayno, get_current_time(), buff.offset,buff.pad+1);
	
	if ((recv_len = recvfrom(temp_s, &buff1, sizeof(ACK_PKT), 0, (struct sockaddr *) &server_address, &slen)) == -1)
        {
            die("recvfrom()");
        }
		printf("RELAY%d\tR\t%s\tACK\t%d\tSERVER\tRELAY%d\n\n", relayno, get_current_time(), buff.offset,buff.pad+1);

	if (sendto(s, &buff1, sizeof(ACK_PKT), 0, (struct sockaddr*) &si_other, slen) == 0)
        {
            die("sendto()");
        }
		printf("RELAY%d\tS\t%s\tACK\t%d\tRELAY%d\tCLIENT\n\n", relayno, get_current_time(), buff.offset,buff.pad+1);
	
}
else{
	printf("RELAY%d\tD\t%s\tDATA\t%d\tRELAY%d\tSERVER\n\n", relayno, get_current_time(), buff.offset,buff.pad+1);
}
    }
    close(s);
}


 
int main(void)
{
    pthread_t relay1,relay2;
    int int0 = 0;
	int int1 = 1;
    pthread_create(&relay1, NULL, relay_sock, &int0);
    pthread_create(&relay2, NULL, relay_sock, &int1);
    pthread_join(relay1, NULL);
    pthread_join(relay2, NULL);
    return 0;
}

