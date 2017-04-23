/*
Yaxi Yu
V00828218
csc361 Program2
Reliable Datagram Protocol (based on UDP)
2017 March
*/
//====================================include======================================================
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
//===================================global variable================================================
//input info
char *sender_ip;
int sender_port;
char *receiver_ip;
int receiver_port;
char *sender_file_name;

//socket info
int sock;			//socket
struct sockaddr_in Sendersa; 	//sender IP
struct sockaddr_in Receiversa;	//receiver IP
socklen_t Sfromlen;
socklen_t Rfromlen;

//header info
char header[1024];
char event_type ='s';	  
char *packet_type ="SYN";	
char tmp[8];
int seqno=0;
int ackno=0;
int payload=0;
int window=0;

//file info
FILE *file;
int file_size;

//summary
struct Summary summary;  

//calc total time duration
clock_t begin, end;

//=====================================struct======================================================
struct Summary{
    int totaldatasent;
    int dataresent;
    int totalpacketssent;
    int packetsresent;
    int syn;
    int fin;
    int rstsent;
    int ack;
    int rstreceive;
    double totaltimeduration;
};

//=====================================methods======================================================

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
    	 printf("total data bytes sent: %i\n", summary.totaldatasent);
	 printf("unique data bytes sent: %i\n", summary.totaldatasent - summary.dataresent);
         printf("total data packets sent: %i\n", summary.totalpacketssent);
         printf("unique data packets sent: %i\n", summary.totalpacketssent - summary.packetsresent);
         printf("SYN packets sent: %i\n", summary.syn);
         printf("FIN packets sent: %i\n", summary.fin);
         printf("RST packets sent: %i\n", summary.rstsent);
         printf("ACK packets received: %i\n", summary.ack);
         printf("RST packets received: %i\n", summary.rstreceive);
	 printf("total time duration (second): %f\n", summary.totaltimeduration);
}


int sendMessage(){
	int bytes;
	bytes = sendto(sock, header, 1024, 0,(struct sockaddr*)&Receiversa, sizeof Receiversa);
	if (bytes<0) 
	{
		perror("rdps: error on sendto()!\n");
		exit(EXIT_FAILURE);
     	}
	//printf("header sent: %.*s\n", bytes, header);
	return bytes;
}

int receiveMessage(){
	int bytes;
	bytes = recvfrom(sock, header,1024, 0, (struct sockaddr *)&Receiversa, &Rfromlen);
	if (bytes<0) 
	{
		perror("rdps: error on recvfrom()!\n");
		exit(EXIT_FAILURE);
     	}	
	//printf("header received: %.*s\n", bytes, header);
	return bytes;
}

void printLogMessage(){
	printtime();
	printf("%c %s:%i %s:%i %s %i %i\n",event_type,sender_ip,sender_port,receiver_ip,receiver_port,packet_type,seqno,payload);
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


//======================================main========================================================
int main(int argc, char **argv)
{
	//start counting time
	begin = clock();
	/*
		./rdps sender_ip sender_port receiver_ip receiver_port sender_file_name
		./rdps 192.168.1.100 8080 10.10.1.100 8080 sent.dat
		   0         1        2      3         4      5
	*/

	//==============check input========================
	if (argc != 6){
		printf("Incorrect number of arguments! sample input: \n");
		printf("./rdps sender_ip sender_port receiver_ip receiver_port sender_file_name");
		return EXIT_FAILURE;
	}
 	//================read input=======================
	sender_ip = argv[1];
	sender_port = atoi(argv[2]);
	receiver_ip = argv[3];
	receiver_port = atoi(argv[4]);
	sender_file_name = argv[5];

	//================socket===========================
	//create socket
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock<0) {
	    printf("rdps: error on socket()");
	    exit(EXIT_FAILURE);
	}

	//set for reuse
	int optval =1;
	if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval))<0){
		close(sock);
		perror("setsockopt error!\n");
		exit(EXIT_FAILURE);	
	}

	//set the memory
       	bzero((char *)&Sendersa, sizeof(Sendersa));     
	bzero((char *)&Receiversa, sizeof(Receiversa)); 

	//sender's IP &PORT
	memset(&Sendersa, 0, sizeof Sendersa);
	Sendersa.sin_family = AF_INET;
	Sendersa.sin_addr.s_addr = inet_addr(sender_ip);	//192.168.1.100
	Sendersa.sin_port = htons(sender_port);			//8080
	Sfromlen = sizeof(Sendersa);
	//receiver's IP & PORT
	memset(&Receiversa, 0, sizeof Receiversa);
	Receiversa.sin_family = AF_INET;
	Receiversa.sin_addr.s_addr = inet_addr(receiver_ip);	//10.10.1.100
	Receiversa.sin_port = htons(receiver_port);		//8080
	Rfromlen = sizeof(Receiversa);

	//bind
	if (bind(sock, (struct sockaddr *)&Sendersa, sizeof(Sendersa)) < 0)
	{
		perror("rdps: Error on binding");
		return -1;
	}


	//===========initialize header=======================
	memset (header, '-' ,  83);//set first 90 bytes to '-'
	memcpy (&header[0], "magic", 5);
	memcpy (&header[5], "CSC361", 6);
	memcpy (&header[11], "DAT", 3);
	memcpy (&header[14], "0", 1);	//DAT=0
	memcpy (&header[15], "ACK", 3);
	memcpy (&header[18], "0", 1);	//ACK=0
	memcpy (&header[19], "SYN", 3);
	memcpy (&header[22], "0", 1);	//SYN=0
	memcpy (&header[23], "FIN", 3);
	memcpy (&header[26], "0", 1);	//FIN=0
	memcpy (&header[27], "RST", 3);
	memcpy (&header[30], "0", 1);	//RST=0
	memcpy (&header[31], "seqno", 5);
	memcpy (&header[36], "0", sizeof(char));//seqnumber    8 bytes
	memcpy (&header[44],"ackno",5);
	memcpy (&header[49], "0", sizeof(char));//acknumber    8 bytes
	memcpy (&header[57],"length",6);
	memcpy (&header[63], "0", sizeof(char));//payload      8 bytes      
	memcpy (&header[71],"size",4);
	memcpy (&header[75], "0", sizeof(char));//window size  8 bytes

	/*header totoal 84 bytes: from header[0] to header[83]	
	header[84] to header[1023] should be payload to transfer file*/

	memcpy (&header[22], "1", 1);	// first set SYN=1
	/*
	printf("initial sequence number: %c\n", header[36]);
	printf("initial acknumber number: %c\n", header[49]);
	printf("initial payload: %c\n", header[63]);
	printf("initial window size: %c\n", header[75]);
	*/
	//=============open&read file======================
	file=fopen(sender_file_name,"rb"); 
	if(file == NULL){
		printf("rdps: file not exist\n");
		exit(EXIT_FAILURE);
	}
	//read file size
	fseek (file, 0L, SEEK_END);
	file_size = ftell(file);

	
	int dataToTrans=file_size;





	//=============transmitting file======================
	

	//select() instruction from http://blog.csdn.net/piaojun_pj/article/details/5991968
	fd_set readfds;//set of socket descriptor
	fd_set testfds;
	int result;
	FD_ZERO(&readfds);//clear the socket set
	FD_SET(sock, &readfds);//add sender's socket to the set	
	struct timeval timeout;//timeout 
	//this while loop is after the connection is established

	if((header[22]=='1')&&(header[18]=='0')){// send first SYN
				packet_type = "SYN";
				event_type = 's';
				summary.syn++;
				printLogMessage();
				int bytes_sent = sendMessage();				
	}

	while(1){
		testfds = readfds;
		timeout.tv_sec=0;
		timeout.tv_usec=500000;
		//wait for an activity on one of the sockets
		result = select(sock+1,&testfds, (fd_set *)0, (fd_set *)0, &timeout);
		switch(result){
			case 0://no response from the receiver
				if((header[22]=='1')&&(header[18]=='0')){//if last packet is SYN
					event_type = 'S';
					packet_type = "SYN";
					summary.syn++;
					printLogMessage();
					int bytes_sent = sendMessage();	
					break;			
				}
				if((header[14]=='1') &&(header[18]=='0')){//if last packet is DAT
					event_type = 'S';
					packet_type = "RST";
					cleartype();
					header[30]='1';
					printLogMessage();
					summary.rstsent++;
					int bytes_send = sendMessage();
					break;
				}
				if((header[26]=='1') &&(header[18]=='0')){//if last packet is FIN
					event_type = 'S';
					packet_type = "FIN";
					printLogMessage();
					int bytes_send = sendMessage();
					break;
				}
				break;

			case -1:
				perror("rdps: Failed to select!");
				close(sock);
				exit(EXIT_FAILURE);
			default:
				  if(FD_ISSET(sock,&testfds))   
				  {   
        				receiveMessage();//receive header from other side
					readData();//read seqno, ackno, payload, window
					if((header[18]=='1')&&((header[22]=='1')||(header[14]=='1'))){//ACK + (SYN or DAT)
						//receive
						packet_type = "ACK";
						event_type ='r';
						printLogMessage();
						summary.ack++;
						//sendback
						cleartype();
						if(dataToTrans>0){//send DAT
							header[14]='1';
							packet_type = "DAT";
							if(900 < dataToTrans){
								//upload payload
								payload=900;
								//store file content into header
								fseek(file, seqno, SEEK_SET);//scan file from seqno
								size_t newLen = fread(&header[100], sizeof(char), payload, file);//scan 900
								if (newLen == 0) {
								    fputs("Error reading file", stderr);
								}
								//update payload
								char Payload[8];
								sprintf(Payload, "%d", payload);
								memcpy(&header[63], Payload, strlen(Payload));
								//update seqno
								seqno = seqno + payload;
								char Seqno[8];
								sprintf(Seqno, "%d", seqno);
								memcpy(&header[36], Seqno, strlen(Seqno));
								//send header to receiver
								int bytes_send = sendMessage();
								event_type = 's';
								printLogMessage();
								summary.totaldatasent+=payload;
								dataToTrans -=payload;	
							}else{
							        //store data into header
								fseek(file, seqno, SEEK_SET);//scan file from seqno
								size_t newLen = fread(&header[100], sizeof(char), dataToTrans, file);//scan remain
								if (newLen == 0) {
								    fputs("Error reading file", stderr);
								}
								header[100+dataToTrans]='\0';//update header info
								//update payload
								char Payload[8];
								sprintf(Payload, "%d", dataToTrans);
								memcpy(&header[63], Payload, strlen(Payload));
								//update seqno
								seqno = seqno + dataToTrans;
								char Seqno[8];
								sprintf(Seqno, "%d", seqno);
								memcpy(&header[36], Seqno, strlen(Seqno));
								//send header to receiver
								int bytes_send = sendMessage();
								event_type = 's';
								printLogMessage();
								summary.totaldatasent+=dataToTrans;
								dataToTrans = 0;
							}
							summary.totalpacketssent++;	
						}else{//send FIN
							header[26]='1';
							packet_type = "FIN";
							int bytes_send = sendMessage();
							event_type = 's';
							printLogMessage();
							summary.fin++;
							summary.totalpacketssent++;	
						} 
					}


					if(header[18]=='1' && header[26]=='1'){// ACK + FIN
						//TODO:finish, print summary
						//receive
						packet_type = "ACK";
						event_type ='r';
						printLogMessage();
						summary.ack++;
						end = clock();
						summary.totaltimeduration = (double)(end - begin) / CLOCKS_PER_SEC;
						printSummary();
						break;
					}
						
				  }//FD_ISSET end 
				  break;
			}//switch select end
	}//while loop end	
	close(sock);
	return 0;
}



