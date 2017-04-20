#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define BUFLEN 100
#define SERVPORT 29876
#define MAXLINE 100

int main(int argc, char** argv)
{
        int sockfd, sendbytes;
        char send[MAXLINE];
        char buffer[BUFLEN];
        struct hostent *host;
        struct sockaddr_in servaddr;

        if ((host = gethostbyname(argv[1])) == NULL) {
                perror("Gen host name failed\n");
                exit(EXIT_FAILURE);
        }
        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
                printf("Create socket failed, errno: %d\n", errno);
                exit(EXIT_FAILURE);
        }

        /* configure socket */
        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr = *((struct in_addr *)host->h_addr);;
        servaddr.sin_port = htons(SERVPORT);
        bzero(&servaddr.sin_zero, 8);

        if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr)) == -1) {
                printf("Connect server failed\n");
                exit(EXIT_FAILURE);
        }
        while (fgets(send, 1024, stdin) != NULL) {
                if ((sendbytes = write(sockfd, send, 100)) == -1) {
                        perror("Send error \n");
                        exit(EXIT_FAILURE);
                }
        }
        close(sockfd);
}

