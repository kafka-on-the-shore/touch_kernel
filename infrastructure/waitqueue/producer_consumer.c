#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/slab.h>

static struct task_struct *producer = NULL;
static struct task_struct *consumer = NULL;
static wait_queue_head_t prod_wq, cons_wq;

struct my_work {
    char name[64];
    void (*work_func)(void *data);
    void *data;
};

static struct my_work *work = NULL;


static void do_work(void *data)
{
    int num = (int)data;
    printk(KERN_NOTICE "do work number %d\n", num);
    msleep_interruptible(1000);
}

static int producer_thr(void *arg)
{
    int num = 0;
    printk(KERN_NOTICE "Enter %s()", __func__);
    while (!kthread_should_stop()) {
        int ret = wait_event_interruptible(prod_wq, (work == NULL));
        if (ret == -ERESTARTSYS) {
            printk(KERN_NOTICE "Wake up by signal in %s()", __func__);
            continue;
        }

        work = kzalloc(sizeof(struct my_work), GFP_KERNEL);
        if (!work) {
            printk(KERN_ERR "kzalloc failed");
            break;
        }
        num++;
        snprintf(work->name, sizeof(work->name), "debug-work");
        work->work_func = do_work;
        work->data = (void *)num;

        wake_up_interruptible(&cons_wq);
    }

    printk(KERN_NOTICE "Exit %s()", __func__);
    return 0;
}

static int consumer_thr(void *arg)
{
    printk(KERN_NOTICE "Enter %s()", __func__);
    wake_up_interruptible(&prod_wq);
    
    while (!kthread_should_stop()) {
        int ret = wait_event_interruptible(cons_wq, (work == NULL));
        if (ret == -ERESTARTSYS) {
            printk(KERN_NOTICE "Wake up by signal in %s()", __func__);
            continue;
        }
        
        printk(KERN_NOTICE "Excute work: %s", work->name);
        work->work_func(work->data);
        kfree(work);
        work = NULL;
        wake_up_interruptible(&prod_wq);
    }
    
    printk(KERN_NOTICE "Exit %s()", __func__);
    return 0;
}
static int __init wq_demo_init(void)
{
    printk(KERN_NOTICE "Enter %s()", __func__);
    init_waitqueue_head(&prod_wq);
    init_waitqueue_head(&cons_wq);

    producer = kthread_run(producer_thr, NULL, "producer-thr");
    if (!producer) {
        printk(KERN_ERR "kthread run failed");
        goto _fail;
    }

    producer = kthread_run(consumer_thr, NULL, "consumer-thr");
    if (!consumer) {
        printk(KERN_ERR "kthread run failed");
        goto _fail;
    }

    printk(KERN_NOTICE "Exit %s()", __func__);
    return 0;

_fail:
    if (producer)
        kthread_stop(producer);
    if (consumer)
        kthread_stop(consumer);
    return -ECHILD;
}

static void __exit wq_demo_exit(void)
{
    printk(KERN_NOTICE "Enter %s()", __func__);
    if (producer)
        kthread_stop(producer);
    if (consumer)
        kthread_stop(consumer);
    printk(KERN_NOTICE "Exit %s()", __func__);
}

module_init(wq_demo_init);
module_exit(wq_demo_exit);
MODULE_LICENSE("GPL");
