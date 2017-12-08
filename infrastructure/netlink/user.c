#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <linux/netlink.h>
#include <linux/socket.h>
#include <unistd.h>

#define MAX_PAYLOAD 20480

struct my_message {
    unsigned int    id;
    unsigned int    data_len;
    char            msg[0];
};

static int create_socket(void)
{
    int fd;
    struct sockaddr_nl my_addr;

    fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
    if (fd < 0) {
        printf("create socket failed\n");
        return -1;
    }

    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.nl_family = AF_NETLINK;
    my_addr.nl_pid = getpid(); /* get current thread PID */
    my_addr.nl_groups = 0;

    if (bind(fd, (struct sockaddr*)&my_addr, sizeof(struct sockaddr)) < 0) {
        printf("bind socket failed\n");
        return -1;
    }

    return fd;
}

static void send_message(int fd, struct sockaddr_nl dest_addr,
                struct my_message *msg)
{
    struct msghdr msghdr;
    struct iovec iov;
    /* todo:  how many we need to allocate ? */
    struct nlmsghdr *nlh = (struct nlmsghdr*)malloc(NLMSG_SPACE(MAX_PAYLOAD));

    nlh->nlmsg_len = msg->data_len + sizeof(struct my_message);
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags = 0;
    memcpy(NLMSG_DATA(nlh), msg->msg, msg->data_len);

    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;

    memset(&msghdr, 0, sizeof(msghdr));
    msghdr.msg_name = (void*)&dest_addr;
    msghdr.msg_namelen = sizeof(dest_addr);
    msghdr.msg_iov = &iov;
    msghdr.msg_iovlen = 1;

    sendmsg(fd, &msghdr, 0);
}

int main(int argc, char *argv[])
{
    struct sockaddr_nl dest_addr;
    struct my_message *msg;
    char hello[] = "hello netlink!";

    int fd = create_socket();

    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0; /* for linux kernel */
    dest_addr.nl_groups = 0;

    msg = malloc(sizeof(struct my_message) + strlen(hello));
    strcpy(msg->msg, hello);
    msg->data_len = strlen(hello);
    msg->id = 2017;
    


    for (;;) {
        printf("%s\n", msg->msg);
        send_message(fd, dest_addr, msg);
        sleep(2);
    }
    
    return 1;
}
    
