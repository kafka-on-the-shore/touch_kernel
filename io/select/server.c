#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#undef __FD_SETSIZE
#define __FD_SETSIZE 0x500000
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
        int ready;
        int client[FD_SIZE]; /* save connected fd from client */
        int len;
        ssize_t n;
        fd_set rset, allset;
        char buffer[BUF_LEN];
        struct sockaddr_in servaddr;

        printf("Start server\n");
        if ((listenfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
                printf("Create socket failed, errno: %d\n", errno);
                exit(EXIT_FAILURE);
        }
        printf("listen fd is %d\n", listenfd);

        /* configure socket */
        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_UNIX;
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
                client[i] = -1;  /* clean client waith queue, set to -1. */

        FD_ZERO(&allset);
        FD_SET(listenfd, &allset); /* tell system that we are interested in this fd, notify us if happend */

        while (1) {
                rset = allset;
                if ((ready = select(maxfd + 1, &rset, NULL, NULL, NULL)) == -1) {
                        printf("Listen failed, errno: %d\n", errno);
                        exit(EXIT_FAILURE);
                }

                /* recv a un-interested connection */
                if (ready <= 0)
                        continue;

                if (FD_ISSET(listenfd, &rset)) {
                        printf("Start accpet ... \n");
                        if ((connfd == accept(listenfd, NULL, 0)) == -1 ) {
                                printf ("Accept Error: %d\n", errno);
                                continue;
                        }

                        printf("Accept fd: %d\n", connfd);
                        for (i = 0; i < FD_SIZE; i++) {
                                if (client[i] < 0 ) {
                                        client[i] = connfd; /* save a connection to a empty postion */
                                        break;
                                }
                        }

                        printf("Save fd in postion %d in client array\n", i);
                        if (connfd > FD_SETSIZE) {
                                printf("connfd is %d, larger that FD_SETSIZE %d\n", connfd, FD_SETSIZE);
                        }
                        if (i == FD_SIZE) {
                                printf("Too many ...\n");
                                close(connfd);
                                continue;
                        }

                        FD_SET(connfd, &allset); /* tell system that we are interested to this connection too */

                        if (connfd > maxfd)
                                maxfd = connfd;

                        if (i > maxi)
                                maxi = i;
                }

       

                printf("maxi: %d\n", maxi);
                /* data process */
                for (i = 0; i <= maxi; i++) {
                        /* located the connection */
                        if ((sockfd = client[i]) <= 0)
                                continue;
                        printf("locate fd in postion %d in client array, sockfd is %d\n", i, client[i]);
                        if (FD_ISSET(client[i], &rset) == 0)
                                continue;

                        printf("valid fd in postion %d in client array, sockfd is %d\n", i, client[i]);
                        sockfd = client[i];

                        memset(buffer, 0, sizeof(buffer));
                        n = read(sockfd, buffer, BUF_LEN);
                        printf("socket recv n: %d\n", n);
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

                        printf("locate fd in postion %d in client array, sockfd is %d\n", i, client[i]);
                        sockfd = client[i];

                        memset(buffer, 0, sizeof(buffer));
                        n = read(sockfd, buffer, BUF_LEN);
                        printf("socket recv n: %d\n", n);
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

                        printf("Server recv len: %d, string: %s\n", n,  buffer);

                        if (strcmp(buffer, "q") == 0) {
                                close(sockfd);
                                FD_CLR(sockfd, &allset);
                                client[i] = 1;
                                continue;
                        }

                        printf("Server send: %s\n", buffer);
                        write(sockfd, buffer, n);

                }
        }
        return 0;
}

