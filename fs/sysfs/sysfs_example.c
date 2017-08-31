#include <linux/module.h>
#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/fs.h>
#include <linux/string.h>

static struct kobject *demo_kobj;
static int foo;

static ssize_t foo_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", foo);
}

static ssize_t foo_store(struct kobject *kobj, struct kobj_attribute *attr, char *buf, size_t len)
{
    sscanf(buf, "%d", &foo);
    return len;
}

static struct kobj_attribute foo_attribute = __ATTR(foo, 0660, foo_show, foo_store);


static int __init sysfs_example_init(void)
{
    int error = 0;

    printk(KERN_INFO "Init sysfs example module\n");
    demo_kobj = kobject_create_and_add("kobject_example", kernel_kobj); //note!
    if (!demo_kobj)
        return -ENOMEM;

    error = sysfs_create_file(demo_kobj, &foo_attribute.attr);
    if (error)
        printk(KERN_ERR "Failed to create foo file in /sys/kernel/kobject\n");

    return error;
}

static void __exit sysfs_example_exit(void)
{
    printk(KERN_INFO "Exit sysfs example module\n");
    kobject_put(demo_kobj);
}

module_init(sysfs_example_init);
module_exit(sysfs_example_exit);
MODULE_LICENSE("GPL");
