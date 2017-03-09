#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUF_LEN 1024
#define SERV_PORT 29876
#define FD_SIZE 128
#define MAX_BACK 100

int main(int argc, char** argv)
{
        int listenfd, connfd, sockfd, maxfd, maxi, i;
        int ready, client[FD_SIZE];
        int len;
        ssize_t n;
        fd_set rset, allset;
        char buffer[BUF_LEN];
        socklen_t clilen;
        struct sockaddr_in servaddr, chiaddr;

        if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
                printf("Create socket failed, errno: %d\n", errno);
                exit(EXIT_FAILURE);
        }

        /* configure socket */
        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        servaddr.sin_port = htons(SERV_PORT);

        /* bind */
        if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
                printf("Bind failed, errno: %d\n", errno);
                exit(EXIT_FAILURE);
        }

        /* listen */
        if (listen(listenfd, MAX_BACK) == -1) {
                printf("Listen failed, errno: %d\n", errno);
                exit(EXIT_FAILURE);
        }

        maxfd = listenfd;
        maxi = -1;

        for (i = 0; i < FD_SIZE; i++)
                client[i] = -1;

        FD_ZERO(&allset);
        FD_SET(listenfd, &allset);

        while (1) {
                rset = allset;
                if ((ready = select(maxfd + 1, &rset, NULL, NULL, NULL)) == -1) {
                        printf("Listen failed, errno: %d\n", errno);
                        exit(EXIT_FAILURE);
                }

                if (ready <= 0)
                        continue;

                if (FD_ISSET(listenfd, &rset)) {
                        clilen = sizeof(chiaddr);
                        printf("Start accpet ... \n");
                        if ((connfd == accept(listenfd, (struct sockaddr *)&chiaddr, &clilen)) == -1 ) {
                                printf ("Accept Error: %d\n", errno);
                                continue;
                        }

                        for (i = 0; i < FD_SIZE; i++) {
                                if (client[i] < 0 ) {
                                        client[i] = connfd;
                                        break;
                                }
                        }

                        if (i == FD_SIZE) {
                                printf("Too many ...\n");
                                close(connfd);
                                continue;
                        }

                        FD_SET(connfd, &allset);

                        if (connfd > maxfd)
                                maxfd = connfd;

                        if (i > maxi)
                                maxi = i;
                }

        }

        /* data process */
        for (i = 0; i < maxi; i++) {
                if (client[i] <= 0)
                        continue;

                
                if (!FD_ISSET(sockfd, &rset))
                        continue;

                memset(buffer, 0, sizeof(buffer));
                n = read(sockfd, buffer, BUF_LEN);
                if (n < 0) {
                        printf("Read socket %d error!\n", sockfd);
                        close(sockfd);
                        FD_CLR(sockfd, &allset);
                        client[i] = 1;
                        continue;
                }
                if (n == 0) {
                        printf("Read socket %d end now.\n", sockfd);
                        close(sockfd);
                        FD_CLR(sockfd, &allset);
                        client[i] = 1;
                        continue;
                }

                printf("Server recv: %s\n", buffer);

                if (strcmp(buffer, "q") == 0) {
                        close(sockfd);
                        FD_CLR(sockfd, &allset);
                        client[i] = 1;
                        continue;
                }

                printf("Server send: %s\n", buffer);
                write(sockfd, buffer, n);

        }
        return 0;
}

