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
#define PORT_S 8880   //The port on which to listen for incoming data


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

FILE *fp;
int end = 0;

int main(void)
{
	fp = fopen("output.txt", "w");
	
    struct sockaddr_in si_me, si_other;
    int s, i, slen = sizeof(si_other) , recv_len;
    char buf[BUFLEN];
     
    //create a UDP socket
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
     
    // zero out the structure
    memset((char *) &si_me, 0, sizeof(si_me));
     
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT_S);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
     
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
		printf("SERVER\tR\t%s\tDATA\t%d\tRELAY%d\tSERVER\n\n", get_current_time(), buff.offset, buff.pad+1);
		if(end<buff.offset){
			fseek(fp, end, SEEK_SET);
			char tmp_sp[buff.offset - end];
			memset(tmp_sp, ' ', sizeof(tmp_sp));
			fprintf(fp, "%s", tmp_sp);
			end = buff.offset;
		}
		fseek(fp, buff.offset, SEEK_SET);
		fprintf(fp,"%s", buff.data);
         
        //print details of the client/peer and the data received
        //printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
        //printf("Data: %d  %s\n" , buff.offset, buff.data);
	
	buff1.offset = buff.offset;
	
	if (sendto(s, &buff1, sizeof(ACK_PKT), 0, (struct sockaddr*) &si_other, slen) == -1)
        {
            die("sendto()");
        }
	printf("SERVER\tS\t%s\tACK\t%d\tSERVER\tRELAY%d\n\n", get_current_time(), buff1.offset,buff1.pad+1);

         
        //now reply the client with the same data

        //if (sendto(s, buf, recv_len, 0, (struct sockaddr*) &si_other, slen) == -1)
        //{
            //die("sendto()");
        //}
    }
    close(s);
    return 0;
}

