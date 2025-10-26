#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
//this header file contains definitions of a number of data types used in system calls
#include <sys/types.h>
//the header file socket.h includes a number of definitions of structures needed for sockets. Eg: defines the sockaddr structure
#include <sys/socket.h>
//the header file in.h contains constants ans structures needed for internet domains addresses. Eg: sockaddr_in(We will be using this structure.)
#include <netinet/in.h>

void error(const char *msg){
    perror(msg);
    exit(1);
}

int main(int argc, int *argv[]){

    if(argc < 2){
        fprintf(stderr,"Port number not provided. Program terminated\n");
        exit(1);
    }

    int sockfd, newsockfd, portno,n;
    int buffer[255];

    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;

    sockfd = socket(AF_INET,SOCK_STREAM,0); 

    if(sockfd<0){
        error("Error opening Socket.");
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0);
        error("Blinding Failed.");

    return 0;
}