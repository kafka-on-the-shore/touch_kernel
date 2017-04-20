#include <linux/module.h>
#include <linux/workqueue.h>

struct my_struct_t {
	char * name;
	struct work_struct my_work;
};

void my_queued_job(struct work_struct *work)
{
    // remove the coupling of workqueue with user data by container_of.
	struct my_struct_t *my_data = container_of(work, struct my_struct_t, my_work);
	printk(KERN_INFO "Hello world work queue, my name is %s\n", my_data->name);
}

struct workqueue_struct *my_wq; 
struct my_struct_t my_job;

static int __init wq_init_module(void) {
	printk(KERN_INFO "Start wq module now...\n");
	my_wq = create_workqueue("Rock for WQ");

	my_job.name = "Rock Li";
	INIT_WORK(&(my_job.my_work), my_queued_job);

	queue_work(my_wq, &(my_job.my_work));
	return 0;
}

static void __exit wq_exit_module(void) {
	printk("Destroy my workqueue now...\n");
	destroy_workqueue(my_wq);
	printk("Level wq module now\n");
}

module_init(wq_init_module);
module_exit(wq_exit_module);
MODULE_LICENSE("GPL");

