#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <arpa/inet.h>
#include "thpool.h"

#define BACKLOG_SIZE 10

void dbg( const char * str) {
	std::cerr << str << std::endl;
}

void dbg( const char * str, int n) {
	std::cerr << str << n << std::endl;
}

int init_listening_socket(int port) {
	//Create socket
	int sockfd = socket(AF_INET , SOCK_STREAM , 0);
	if (sockfd == -1) {
		dbg("Could not create socket");
		return -1;
		}

	//Prepare the sockaddr_in structure
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(port);

	//Set reuse option
	int yes=1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
		dbg("Could not set socket option");
		return -1;
		}	
	//Bind
	if( bind(sockfd,(struct sockaddr *)&server , sizeof(server)) < 0) {
		//print the error message
		dbg("bind failed. Error");
		return -1;
		}
	listen(sockfd, BACKLOG_SIZE); 
	return sockfd;
}


void stupid_msg(int fd) {
	const char buf[]="Hello world!\n";
	write(fd,buf,sizeof(buf)-1);
	close(fd);
}

extern void milter_talk(int fd);

void mainloop(int sockfd, thpool *pool) {
	struct sockaddr_in cli_addr;
	socklen_t clilen = sizeof(cli_addr);

	for (;;) {
		int newfd=accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		dbg("CONNESSIONE");
		if (newfd>0)
			pool->push_connection(newfd);
		else {
			dbg("ACCEPT ERRNO: ",errno);
			sleep(2);
			_exit(0);
		}
	}
}

int main(int argc, char **argv) {
	int s=init_listening_socket(8991);
	if (s<0) return -1;
	//thpool p(stupid_msg);
	thpool p(milter_talk);
	p.startThread();
	p.startThread();
	mainloop(s,&p);
	return 0;
}
