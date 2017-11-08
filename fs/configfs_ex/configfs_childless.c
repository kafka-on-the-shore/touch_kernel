
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>

#include <linux/configfs.h>

/*
 * childless example: foo
 *
 * Can not create any config_items, it just has attribute
 *
 * Reference: https://lwn.net/Articles/149007
 */

struct foo {
   struct configfs_subsystem subsys;
   int counter;
   int reg;
};


// An OOP desgin for deal with each attribute, all items of this obj support show/store methods
struct foo_attribute {
    struct configfs_attribute attr;
    ssize_t (*show)(struct foo *, char*);
    ssize_t (*store)(struct foo *, const char *, size_t);
};

//cause foo include subsys, so foo itself is teared from a config_item
static inline struct foo *to_foo(struct config_item *item)
{
    return item ? container_of(to_configfs_subsystem(to_config_group(item)), struct foo, subsys) : NULL;
}

/* attribute: foo/counter */
static ssize_t foo_counter_read(struct foo *foo, char *page)
{
    ssize_t pos;

    pos = sprintf(page, "%d\n", foo->counter);
    foo->counter++;
    printk(KERN_INFO "counter read: %d\n", foo->counter);

    return pos;
}
//attribute under the foo. The dir corresponds to a kernel object, while the attribute under it corresponds a operation to it.
static struct foo_attribute foo_attr_counter = {
    .attr = {
        .ca_owner = THIS_MODULE,
        .ca_name = "counter",
        .ca_mode = S_IRUGO
    },
    .show = foo_counter_read,
};

/* attribute: foo/reg */
static ssize_t foo_reg_read(struct foo *foo, char *page)
{
    printk(KERN_INFO "reg read: %d\n", foo->reg);
    return sprintf(page, "%d\n", foo->reg);
}

static ssize_t foo_reg_write(struct foo *foo, const char *page, size_t count)
{
    unsigned long tmp;
    char *p = (char *)page;

    printk(KERN_INFO "page content: %s\n", page);
    tmp = simple_strtoul(p, &p, 10);
    printk(KERN_INFO "page content after convert: %ld\n", tmp);
    if (!p || (*p && (*p != '\n')))
        return -EINVAL;

    if ( tmp > INT_MAX)
        return -ERANGE;

    foo->reg = tmp;

    return count;
}

static struct foo_attribute foo_attr_reg = {
    .attr = {
        .ca_owner = THIS_MODULE,
        .ca_name = "reg",
        .ca_mode = S_IRUGO | S_IWUSR
    },
    .show = foo_reg_read,
    .store = foo_reg_write,
};

/* attribute: foo/description */
static ssize_t foo_description_read(struct foo *foo, char *page)
{
    return sprintf(page, "Simplest possible subsystem in configfs\n");
}


static struct foo_attribute foo_attr_description = {
    .attr = {
        .ca_owner = THIS_MODULE,
        .ca_name = "description",
        .ca_mode = S_IRUGO
    },
    .show = foo_description_read,
};

// configfs itself just goes here, all above encapsulation is just an OOP-way to deal with attribute of object childess. A good practice.
static struct configfs_attribute *foo_attrs[] = { //->4, KEY:configfs_attribute
    &foo_attr_counter.attr,
    &foo_attr_reg.attr,
    &foo_attr_description.attr,
    NULL
};

static ssize_t foo_attr_show(struct config_item *item,
                    struct configfs_attribute *attr,
                    char *page)
{
    struct foo *foo = to_foo(item);
    struct foo_attribute *foo_attr =
            container_of(attr, struct foo_attribute, attr);
    ssize_t ret = 0;

    if (foo_attr->show)
        ret = foo_attr->show(foo, page);

    return ret;
}

static ssize_t foo_attr_store(struct config_item *item,
                    struct configfs_attribute *attr,
                    const char *page, size_t count)
{
    struct foo *foo = to_foo(item);
    struct foo_attribute *foo_attr =
            container_of(attr, struct foo_attribute, attr);
    ssize_t ret = -EINVAL;

    if (foo_attr->store)
        ret = foo_attr->store(foo, page, count);

    return ret;
}

static struct configfs_item_operations foo_item_ops = { //->4, KEY:configfs_item_operations
    .show_attribute = foo_attr_show,  //common operations for all items in current subsystem
    .store_attribute = foo_attr_store,
};

static struct config_item_type foo_type = { //->3
    .ct_item_ops = &foo_item_ops, //operations for attribute
    .ct_attrs = foo_attrs,        //attributes to be operated
    .ct_owner = THIS_MODULE,
};

static struct foo foo_subsys = { //->2 .subsys is a configfs_group type. Each group belonging to a specific subsystem.
    .subsys = {
        .su_group = {       //KEY: config_group
            .cg_item = {    //KEY: config_item
                .ci_namebuf = "foo",
                .ci_type = &foo_type, //KEY:confi_item_type
            },
        },
    },
};


static struct configfs_subsystem *example_subsys[] = { //->1 top level, a sub systems array. 
    &foo_subsys.subsys, 
    NULL,
};

static int __init configfs_example_init(void)
{
    int ret, i;
    struct configfs_subsystem *subsys;

    for (i = 0; example_subsys[i]; i++) {
        subsys = example_subsys[i];

        config_group_init(&subsys->su_group);
        mutex_init(&subsys->su_mutex);
        ret = configfs_register_subsystem(subsys); //->0
        if (ret) {
            printk(KERN_ERR "Error %d while reging subsystem %s\n",
                    ret,
                    subsys->su_group.cg_item.ci_namebuf);
            goto out_unregister;
        }
    }

    return 0;

out_unregister:
    for (; i >= 0; i--) {
        configfs_unregister_subsystem(example_subsys[i]);
    }

    return ret;
}

static void __exit configfs_example_exit(void)
{
    int i;

    for (i = 0; example_subsys[i]; i++) {
        configfs_unregister_subsystem(example_subsys[i]);
    }
}

module_init(configfs_example_init);
module_exit(configfs_example_exit);
MODULE_LICENSE("GPL");
