/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

/******************************************************************************
**
** BEGIN: USER INCLUDES
**
******************************************************************************/

#include "common.h"
#include "client_main.h"

/******************************************************************************
**
** END: USER INCLUDES 
**
******************************************************************************/


// get sockaddr, IPv4, IPv6:
void* get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char* argv[]) {
    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    
    // Command line argument guard.
    if (argc != 2) { 
        fprintf(stderr, "usage: client hostname\n");
        exit(1);
    }

    // Preparing the IP address lookup struct.
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    // Get the IP address info.
    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("ERROR: socket"); continue;
        }

        // connect() to use recv()
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("ERROR: connect");
            continue;
        }

        break;
    }
    
    // Failed to connect to a listener.
    if (p == NULL) {
        fprintf(stderr, "ERROR: listener: failed to bind socket\n");
        fprintf(stdout, "ERROR: Cannot connect to server\n");
        return 2;
    }

    // Converts the IP address to a string.
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
        s, sizeof(s));

    // Now the IP address is in <s>
    // Print the IP address.
    
    printf("client connecting to %s\n", s);

    freeaddrinfo(servinfo); // all done with this structure

    
    
/******************************************************************************
*******************************************************************************
******************************************************************************/

    /*
    ** client_main() is responsible for the main behavior of the program. 
    */
    int client_main_return = -1;
    if (client_main_return = client_main(sockfd, buf, MAXDATASIZE) != 0) {
        fprintf(
            stderr,
            "ERROR(%d): Abnormal termination of communication with server\n",
            client_main_return
        );
    } else {
        fprintf(
            stdout,
            "OK: Terminated connection normally\n"
        );
    }

/******************************************************************************
*******************************************************************************
******************************************************************************/    close(sockfd);

    return 0;
}
