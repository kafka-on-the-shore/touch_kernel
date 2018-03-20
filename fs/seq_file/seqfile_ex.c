#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/percpu.h>
#include <linux/sched.h>

#define EX_PROC_ENTRY       "ex_seqfile"

/*
 * operations of seq file
 *
 * NOTE: the object to be present is passed during these operations by
 * void *. So the actually state of this object during showing should
 * be maintained by itself.
 */

static void *l_start(struct seq_file *m, loff_t *pos)
{
    loff_t index = *pos;

    if (index == 0) {
        seq_printf(m, "Current all the processes in system:\n"
                      "%-24s%-5s\n", "name", "pid");
        return &init_task;
    } else {
        return NULL;
    }
}

static void *l_next(struct seq_file *m, void *p, loff_t *pos)
{
    struct task_struct *task = (struct task_struct *)p;

    task = next_task(task);
    if ((*pos != 0) && (task == &init_task)) {
        return NULL;
    }
    ++*pos;

    return task;
}

static void l_stop(struct seq_file *m, void *p)
{
}

static int l_show(struct seq_file *m, void *p)
{
    struct task_struct *task = (struct task_struct *)p;

    seq_printf(m, "%-24s%-5d\n", task->comm, task->pid);
    return 0;
}

/*
 * seq_file interface works in Adapter pattern.
 * It passes most of the operations to fpos of proc.
 */
static struct seq_operations ex_seq_ops = {
    .start = l_start,
    .next = l_next,
    .stop = l_stop,
    .show = l_show,
};

static int ex_seq_open(struct inode *inode, struct file *file)
{
    return seq_open(file, &ex_seq_ops);
}

static struct file_operations ex_seq_fops = {
    .open = ex_seq_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = seq_release,
};

static int __init ex_seq_init(void)
{
    struct proc_dir_entry *proc_entry;

    proc_entry = proc_create(EX_PROC_ENTRY, 0, NULL, &ex_seq_fops);
    if (!proc_entry)
        printk(KERN_ERR "create proc file ex_seqfile failed");

    return 0;
}

static void __exit ex_seq_exit(void)
{
    remove_proc_entry(EX_PROC_ENTRY, NULL);
}

module_init(ex_seq_init);
module_exit(ex_seq_exit);
MODULE_LICENSE("GPL");
