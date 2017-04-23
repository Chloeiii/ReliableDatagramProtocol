/*
Yaxi Yu
V00828218
csc361 Program2
Reliable Datagram Protocol (based on UDP)
2017 March
*/
//=========================================include==============================================
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h> /* for close() for socket */ 
#include <stdlib.h>
#include <time.h>
//=====================================global variable============================================
//input info
char *receiver_ip;
int receiver_port;
char *receiver_file_name;
struct sockaddr_in Receiversa; 	//receiver IP
struct sockaddr_in Sendersa; 	//sender IP


//socket info
int sock;			
ssize_t recsize;
socklen_t fromlen;

char header[1024];
char event_type ='r';	  	
char *packet_type ="ACK";	

int seqno=0;
int ackno=0;
int payload;
int window;//each time can store 5 full packets
char tmp[8];

//file info
FILE *file;
int file_size;

//summary
struct Summary summary;  

//calc total time duration
clock_t begin, end;
//==========================================struct================================================
struct Summary{
    int totaldatareceived;
    int datarereived;
    int totalpacketreceived;
    int packetsrereceived;
    int syn;
    int fin;
    int rstreceive;
    int ack;
    int rstsent;
    double totaltimeduration;
};
//==========================================methods===============================================
void printtime(){
	//hour, min, sec
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	printf ("%d:%d:%d.", tm.tm_hour, tm.tm_min, tm.tm_sec);
	//microsec
	struct timeval curTime;
	gettimeofday(&curTime, NULL);
	printf("%ld: ",curTime.tv_usec);
}
void printSummary(){
    	 printf("total data bytes received: %i\n", summary.totaldatareceived);
	 printf("unique data bytes received: %i\n", summary.totaldatareceived - summary.datarereived);
         printf("total data packets received: %i\n", summary.totalpacketreceived);
         printf("unique data packets received: %i\n", summary.totalpacketreceived - summary.packetsrereceived);
         printf("SYN packets received: %i\n", summary.syn);
         printf("FIN packets received: %i\n", summary.fin);
         printf("RST packets received: %i\n", summary.rstreceive);
         printf("ACK packets sent: %i\n",summary.ack);
         printf("RST packets sent: %i\n", summary.rstsent);
	 printf("total time duration (second): %f\n", summary.totaltimeduration);
}

int receiveMessage(){
	recsize = recvfrom(sock, (void*)header, 1024, 0, (struct sockaddr*)&Receiversa, &fromlen);
	if (recsize < 0) {
		fprintf(stderr, "%s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	//printf("header received: %.*s\n", (int)recsize, header);
	//printf("header copied: %.*s\n", 20, &header[100]);
	return (int)recsize;
}

void printLogMessage(){
	//find the other side ip and port
	int len=20;
	char other_ip[len];
	inet_ntop(AF_INET, &(Receiversa.sin_addr),  other_ip, len);
	
	printtime();
	printf("%c %s:%i %s:%i %s %i %i\n",event_type,receiver_ip,receiver_port,other_ip,receiver_port,packet_type,ackno,window);	
}

int sendMessage(){
	int bytes;
	bytes = sendto(sock, header, 1024, 0,(struct sockaddr*)&Receiversa, sizeof Receiversa);
	if (bytes<0) 
	{
		perror("rdpr: error on sendto()!\n");
		exit(EXIT_FAILURE);
     	}
	//printf("header sent: %.*s\n", bytes, header);
	return bytes;
}

void readData(){//read seqno, ackno, payload, window from the header
	seqno=atoi(memcpy(tmp, &header[36], sizeof(tmp)+1));
	ackno=atoi(memcpy(tmp, &header[49], sizeof(tmp)+1));
	payload = atoi(memcpy (tmp, &header[63], sizeof(tmp)+1)); 
	window = atoi(memcpy (tmp, &header[75], sizeof(tmp)+1));
}

void cleartype(){
        header[14]='0';	//DAT=0
	header[18]='0';	//ACK=0
	header[22]='0';	//SYN=0
	header[26]='0';	//FIN=0
	header[30]='0';	//RST=0
}
//===========================================main=================================================
int main(int argc, char **argv)

{
	//start counting time
	begin = clock();
	/*
		./rdpr receiver_ip receiver_port receiver_file_name
		./rdpr 10.10.1.100     8080         received.dat
		   0         1           2                3         
	*/

	//===================check input===================
	if (argc != 4){
		printf("Incorrect number of arguments! sample input: \n");
		printf("./rdpr receiver_ip receiver_port receiver_file_name\n" );
		//return EXIT_FAILURE;
		exit(-1);
	}
	//====================read input=====================
	receiver_ip = argv[1];
	receiver_port = atoi(argv[2]);
	receiver_file_name = argv[3];
	
	//======================socket========================
	//create a socket
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock<0) {
	    printf("rdpr: error on socket()");
	    exit(EXIT_FAILURE);
	}

	//set the memory
       	bzero((char *)&Sendersa, sizeof(Sendersa));     
	bzero((char *)&Receiversa, sizeof(Receiversa));

	//receiver's IP & PORT
	memset(&Receiversa, 0, sizeof Receiversa);
	Receiversa.sin_family = AF_INET;
	Receiversa.sin_addr.s_addr = inet_addr(receiver_ip);	//10.10.1.100
	Receiversa.sin_port = htons(receiver_port);		//8080
	fromlen = sizeof(Receiversa);

	//set for reuse
	int optval =1;
	if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval))<0){
		close(sock);
		perror("setsockopt error!\n");
		exit(EXIT_FAILURE);	
	}
	//bind
	if (-1 == bind(sock, (struct sockaddr *)&Receiversa, sizeof Receiversa)) {
		perror("rdpr: error bind failed");
		close(sock);
		exit(EXIT_FAILURE);
	}

	//file
 	file = fopen(receiver_file_name, "w");

	//=================transmitting file============================
	fd_set readfds;//set of socket descriptor
	fd_set testfds;
	int result;
	FD_ZERO(&readfds);//clear the socket set
	FD_SET(sock, &readfds);//add socket to the set	
	struct timeval timeout;//timeout 

	while(1){
		testfds = readfds;
		timeout.tv_sec=2;
		timeout.tv_usec=500000;

		result = select(sock +1,&testfds, (fd_set *)0, (fd_set *)0, &timeout);
		switch(result){
			case 0://no response from the sender
				//printf("time out keep waiting\n");
				//keep waiting
				timeout.tv_sec=1;
				timeout.tv_usec=0;
				break;
			case -1:
				perror("rdpr: Failed to select!");
				close(sock);
				exit(EXIT_FAILURE);
			default:
				if(FD_ISSET(sock,&testfds))   
				{   
					int receive = receiveMessage();//receive header from other side
					readData();//store seqnum, acknum, payload, window

					if(header[22]=='1'){//SYN
						//receive
						summary.syn++;
						packet_type = "SYN";
						if(summary.syn=0){
							event_type = 'r';
						}else{
							event_type = 'R';//if more than 1 syn, it must be a resent syn packet
						}
						
						summary.totalpacketreceived++;
						printLogMessage();
						//sendback
						cleartype();
						header[22]='1';//SYN
						header[18]='1';//ACK
							//init payload and window
							payload=0;
							window=4500;//each time can store 5 full packets
							char Payload[8];
							char Window[8];
							sprintf(Payload, "%d", payload);
							memcpy(&header[63], Payload, strlen(Payload));
							sprintf(Window, "%d", window);
							memcpy(&header[75], Window, strlen(Window));
						int sendBack=sendMessage();
						packet_type = "ACK";
						if(summary.syn=0){
							event_type = 's';
						}else{
							event_type = 'S';//if more than 1 syn, it must be a resent syn packet
						}
						summary.ack++;
						printLogMessage();
					}



					if((header[14]=='1') || (header[30]=='1')){//DAT or RST
						//receive
						if(header[14]='1'){//DAT
							packet_type = "DAT";
							event_type ='r';
							summary.totalpacketreceived++;
						}
						
						if(header[30]=='1'){//RST
							packet_type = "RST";
							event_type ='R';
							summary.packetsrereceived++;
							//summary.totalpacketreceived++;
						}
						printLogMessage();
							//read data from header
							char tmpData[900];
							memcpy (tmpData, &header[100], payload);
							//write data into the file
   				 			fseek(file,seqno-payload,SEEK_SET);//set file pointer to seqno
							fwrite (tmpData , sizeof(char), payload, file);
						summary.totaldatareceived+=payload;

				
						//sendback
						cleartype();
						header[14]='1';
						header[18]='1';
						//update ackno
						ackno=seqno;
							char Ackno[8];
							sprintf(Ackno, "%d", ackno);
						memcpy(&header[49], Ackno, 8);
						int sendBack=sendMessage();
						packet_type = "ACK";
						event_type ='s';
						printLogMessage();	
						summary.ack++;
				
					}



					if(header[26]=='1'){//FIN
						//receive
						packet_type ="FIN";
						event_type = 'r';
						printLogMessage();
						//printf("FIN received\n");
						summary.fin++;
						//send
						cleartype();
						header[26]='1';
						header[18]='1';
						packet_type ="ACK";
						event_type = 's';
						printLogMessage();
						summary.ack++;
						int sendBack=sendMessage();
						end = clock();
						summary.totaltimeduration = (double)(end - begin) / CLOCKS_PER_SEC;
						printSummary();
						fclose(file);
						break;
					}
				}//FD_ISSET end
				break;
		}//switch select end
	}//while loop end
	close(sock); 
	return 0;
}

