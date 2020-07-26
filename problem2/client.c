/* Simple udp client */
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
#define PORT2 8882
#define PORT 8880   // The port on which to send data

	
 
    


void die(char *s)
{
    perror(s);
    exit(1);
}

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
	int relayno;
	DATA_PKT data_para;
}para_send;

	struct sockaddr_in relay1, relay2;
    int s, i, rlen1=sizeof(relay1), rlen2=sizeof(relay2);
    char buf[BUFLEN];
    char message[BUFLEN];
	FILE *fp;
	int main_offset = 0;
	int *ack_data;
	int count;
	int end;

void *send_data_relay(void * para){
	para_send *tmp = (para_send*)para;
while(ack_data[tmp->data_para.offset/100]==0 && tmp->data_para.offset<=end){
	if(tmp->relayno == 0){ 
		if (sendto(s, &(tmp->data_para), BUFLEN , 0 , (struct sockaddr *) &relay1, rlen1)==-1)
    	{
    	    die("sendto()");
	    }
		printf("CLIENT\tS\t%s\tDATA\t%d\tCLIENT\tRELAY%d\n\n", get_current_time(), tmp->data_para.offset,tmp->relayno+1); 
	}else{
		if (sendto(s, &(tmp->data_para), BUFLEN , 0 , (struct sockaddr *) &relay2, rlen2)==-1)
    	{
    	    die("sendto()");
	    }
		printf("CLIENT\tS\t%s\tDATA\t%d\tCLIENT\tRELAY%d\n\n", get_current_time(), tmp->data_para.offset,tmp->relayno+1);
	}
	sleep(2);
	if(ack_data[tmp->data_para.offset/100]==0 && tmp->data_para.offset<=end){
			printf("CLIENT\tTO\t%s\tACK\t%d\tRELAY%d\tCLIENT\n\n", get_current_time(), tmp->data_para.offset,tmp->relayno+1);
			printf("CLIENT\tRE\t%s\tDATA\t%d\tCLIENT\tRELAY%d\n\n", get_current_time(), tmp->data_para.offset,tmp->relayno+1);
}
}
	
}
	
void *receive_ack(void * para){
	int relay_no = *(int*)para;
	ACK_PKT buff1;
	if(relay_no == 0){
		while(1){ 
			if (recvfrom(s, &(buff1), BUFLEN , 0 , (struct sockaddr *) &relay1, &rlen1)==-1)
    		{
    		    die("sendto()");
	    	}
			if(buff1.offset/100<=count)
			ack_data[buff1.offset/100] = 1;
			//printf("%d\n", buff1.offset);
			printf("CLIENT\tR\t%s\tACK\t%d\tRELAY%d\tCLIENT\n\n", get_current_time(), buff1.offset,buff1.pad+1); 
		}
	}else{
		while(1){
			if (recvfrom(s, &(buff1), BUFLEN , 0 , (struct sockaddr *) &relay2, &rlen2)==-1)
    		{
    		    die("sendto()");
		    }
			if(buff1.offset/100<=count)
			ack_data[buff1.offset/100] = 1;
			//printf("%d\n", buff1.offset);
			printf("CLIENT\tR\t%s\tACK\t%d\tRELAY%d\tCLIENT\n\n", get_current_time(), buff1.offset,buff1.pad+1);
		}
	}
}

int main(void)
{
	fp = fopen("input.txt", "r");
	//int end;
	//int count;
	fseek(fp, 0, SEEK_END);
	end = ftell(fp);
	count = (int)end/BUFLEN;
	int flag = 0;
	if(end%BUFLEN!=0){
		count++;
	}
	para_send send_array[count];
	ack_data = (int*) malloc(count * sizeof(int));
	memset(ack_data, 0, sizeof(ack_data));
	
	for(int i=0; i<count;i++){
		if(i%2==0){
			send_array[i].relayno = 0;
		}else{
			send_array[i].relayno = 1;
		}
		fseek(fp, main_offset, SEEK_SET);
		fread (send_array[i].data_para.data,sizeof(char),BUFLEN,fp);
		send_array[i].data_para.offset = main_offset;
		main_offset += BUFLEN;
	}
	
	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
        die("socket");
    }
 
    memset((char *) &relay1, 0, sizeof(relay1));
    relay1.sin_family = AF_INET;
    relay1.sin_port = htons(PORT1);
    relay1.sin_addr.s_addr = inet_addr("127.0.0.1");

	memset((char *) &relay2, 0, sizeof(relay2));
    relay2.sin_family = AF_INET;
    relay2.sin_port = htons(PORT2);
    relay2.sin_addr.s_addr = inet_addr("127.0.0.2");
	
	int r0 = 0;
	int r1 = 1;
	pthread_t rcv0;
	pthread_t rcv1;
	pthread_create(&rcv0, NULL, receive_ack, &r0);
	pthread_create(&rcv1, NULL, receive_ack, &r1);
  
    while(1)
    {
        //printf("Enter message : ");
		if(flag>count){
			//exit(1);
			break;
		}
		pthread_t tmp1;
		pthread_create(&tmp1, NULL, send_data_relay, &send_array[flag]);
		flag++;
		
        //send the message
        //if (sendto(s, message, strlen(message) , 0 , (struct sockaddr *) &relay1, rlen1)==-1)
        //{
        //    die("sendto()");
        //}
		//if (sendto(s, message, strlen(message) , 0 , (struct sockaddr *) &relay2, rlen2)==-1)
        //{
        //    die("sendto()");
        //}
		
		


        //receive a reply and print it
        //clear the buffer by filling null, it might have previously received data
        //memset(buf,'\0', BUFLEN);
        //try to receive some data, this is a blocking call
        //if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen) == -1)
        //{
        //    die("recvfrom()");
        //}
         
        //puts(buf);
    }
	bool yes = false;
	bool flag_1 = true;
	while(yes == false){
		flag_1 = true;
		for(int j = 0; j<count;j++){
			if(ack_data[j] == 0){
				flag_1 = false;
			}
		}
		if(flag_1 == true){
			yes = true;
		}
	}
	exit(1);
 	pthread_join(rcv0, NULL);
	pthread_join(rcv1, NULL);
    close(s);
    return 0;
}
