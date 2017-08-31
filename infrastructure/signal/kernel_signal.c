/*
 * Show how to leverage signal in kernel.
 *
 * Insmod will start print logs in messages, when send signal by kill from userspace,
 * the signal will be print and log stops
 * Rmmod will stop the signal process.
 */

#include <linux/init.h>  
#include <linux/module.h>  
#include <linux/signal.h>  
#include <linux/spinlock.h>  
#include <linux/sched.h>  
#include <linux/uaccess.h>  
#include <linux/delay.h>
#include <linux/kthread.h>

static struct task_struct *tsk;

struct sig_desc {
    int signal;
    char description[256];
};

static void my_sigpending(sigset_t *set)
{
    spin_lock_irq(&current->sighand->siglock);
    sigorsets(&set, &current->pending.signal, &current->signal->shared_pending.signal);
    spin_unlock_irq(&current->sighand->siglock);
}

static int thread_process(void *arg)
{
    int i;
    sigset_t *sigset, __sigset;

    sigset = &__sigset;

    struct sig_desc listen_signals[4] = {
        {SIGURG,  "SIGURG"},
        {SIGTERM, "SIGTERM"},
        {SIGKILL, "SIGKILL"},
        {SIGSTOP, "SIGSTOP"},
        {SIGCONT, "SIGCONT"}
    };

    for (i = 0; i < 5; i++) {
        allow_signal(listen_signals[i].signal);
    }

    printk(KERN_ERR "PID of this process: %d\n", current->pid);
    my_sigpending(sigset);

    printk(KERN_ERR "Before recv signal, sigmap: 0x%1X, blockedmap: 0x%1X\n", \
        sigset->sig[0],
        current->blocked.sig[0]);

    while (!kthread_should_stop()) {
        msleep(1000);
        printk(KERN_ERR "->");
        if (signal_pending(current)) {
            my_sigpending(sigset);
            printk(KERN_ERR "current signal set: 0x%1X.\n", sigset->sig[0]);

            for (i = 0; i < 5; i++) {
                if (sigismember(sigset, listen_signals[i].signal)) {
                    printk(KERN_ERR "recv signal: %s\n", listen_signals[i].description);
                }
            }
            break;
        }
    }

    return 0;
}

static int __init signal_test_init(void)
{
    tsk = kthread_run(thread_process, NULL, "kernel_signal_test");
    if (IS_ERR(tsk)) {
        printk(KERN_ERR "create kthread failed\n");
    }

    return 0;
}

static void __exit signal_test_exit(void)
{
    kthread_stop(tsk);
    msleep(2000);
    printk(KERN_ERR "Exit signal test");
}

MODULE_LICENSE("GPL");
module_init(signal_test_init);
module_exit(signal_test_exit);
