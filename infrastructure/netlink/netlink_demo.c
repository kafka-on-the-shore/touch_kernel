#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <net/sock.h>
#include <net/netlink.h>

struct my_message {
    unsigned int    id;
    unsigned int    data_len;
    char            msg[0];
};

static struct sock *nl_sk = NULL;

void nl_data_ready(struct sk_buff *input_skb)
{
    struct sk_buff *skb;
    struct nlmsghdr *nlh;
    struct my_message msg;
    memset(&msg, 0, sizeof(struct my_message));

    skb = skb_get(input_skb);
    if (skb->len >=NLMSG_SPACE(0)) {
        nlh = nlmsg_hdr(skb);
        memcpy(&msg, NLMSG_DATA(nlh), sizeof(msg));

        printk(KERN_INFO "my_message from %d: id->%d, content->%s\n", \
                nlh->nlmsg_pid, 
                msg.id,
                msg.msg);
        kfree_skb(skb);
    }
}

static int setup_netlink(void)
{
    struct netlink_kernel_cfg cfg = {
        .input = nl_data_ready,
    };

    nl_sk = netlink_kernel_create(&init_net, NETLINK_GENERIC, &cfg);
    if (!nl_sk) {
        printk(KERN_ERR "net_link: create netlink socket failed\n");
        return -EIO;
    }

    return 0;
}

static int __init nl_demo_init(void)
{
    int ret;
    ret = setup_netlink();
    printk(KERN_INFO "setup netlink now\n");
    return ret;
}

static void __exit nl_demo_exit(void)
{
    netlink_kernel_release(nl_sk);
    printk(KERN_INFO "release netlink now\n");
}

MODULE_LICENSE("GPL");
module_init(nl_demo_init);
module_exit(nl_demo_exit);

    
