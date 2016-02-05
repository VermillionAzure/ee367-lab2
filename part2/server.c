/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>              /* IO                       */
#include <stdlib.h>             /* STANDARD LIBRARY         */
#include <unistd.h>             /* POSIX DEFINITIONS        */
#include <errno.h>              /* ERROR MACROS w/ STATIC MEMORY LOCATION FOR 
                                   ERRORS */
#include <string.h>             /* STRING MANIPULATION AND MEMORY */
#include <sys/types.h>          /* USED FOR DATA TYPES FOR STANDARD LIBRARY  */
#include <sys/socket.h>         /* PROBABLY THE SOCKET      */
#include <netinet/in.h>         /* INPUT TYPE HEADER FOR INTERNET STUFF */
#include <netdb.h>              /* NETWORK DATABASE OPERATIONS -- LOOKUP 
                                   and DNS I think    */
#include <arpa/inet.h>          /* INTERNET STUFF?? used for host network
                                   endian conversions       */
#include <sys/wait.h>           /* POSIX wait and fork for processes -
                                   Windows problems     */
#include <signal.h>             /* Used for POSIX signals for Linux.    */

#define PORT "3522"

#define BACKLOG 10

void sigchld_handler (int s) {
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;
    
    /*
    ** - Loop until children exist and children have not yet changed state,
    **   or an error occurs.
    ** - Do not block.
    */
    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

// get sockaddr, IPv4 or IPv6
void* get_in_addr(struct sockaddr* sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    } else {
        return &(((struct sockaddr_in6*)sa)->sin6_addr);
    }
}

int main(void) {
    // listen on sock_fd, new connection on new_fd
    int             sockfd, new_fd;
    struct          addrinfo hints, *servinfo, *p;
    // connector's address information
    struct          sockaddr_storage their_addr;     
    socklen_t       sin_size;
    struct sigaction    sa;
    int             yes = 1;
    char            s[INET6_ADDRSTRLEN];
    int             rv;

    // Set the IP address lookup struct data.
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // Get the IP address info.
    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }
        
        // ERROR: setting the soct options didn't work.
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
        
        // ERROR: Could not bind to socket. 
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo);     // all done with this structure

    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");

    while (1) { // main accept() loop
        sin_size = sizeof(their_addr);
        new_fd = accept(sockfd, (struct sockaddr*)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s,
            sizeof(s));
        printf("server: got connection from %s\n", s);

        if (!fork()) { // this is the child process
            close(sockfd);
            if (send(new_fd, "Hello, world!", 14, 0) == -1)
                perror("send");

                close(new_fd);
                exit(0);
        }
        close(new_fd);  // parent doesn't need this

    }

    return 0;
}
