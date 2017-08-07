
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>

#include <linux/configfs.h>

/*
 * childless example
 *
 * Can not create any config_items, it just has attribute
 *
 * Reference: https://lwn.net/Articles/149007
 */

struct childless {
   struct configfs_subsystem subsys; //->
   int showme;
   int storeme;
};


// An OOP desgin for deal with each attribute, all items of this obj support show/store methods
struct childless_attribute {
    struct configfs_attribute attr; //->
    ssize_t (*show)(struct childless *, char*);
    ssize_t (*store)(struct childless *, const char *, size_t);
};

//cause childless include subsys, so childless itself is teared from a config_item
static inline struct childless *to_childless(struct config_item *item)
{
    return item ? container_of(to_configfs_subsystem(to_config_group(item)), struct childless, subsys) : NULL;
}

static ssize_t childless_showme_read(struct childless *childless, char *page)
{
    ssize_t pos;

    pos = sprintf(page, "%d\n", childless->showme);
    childless->showme++;
    printk(KERN_INFO "showme read: %d\n", childless->showme);

    return pos;
}

static ssize_t childless_storeme_read(struct childless *childless, char *page)
{
    printk(KERN_INFO "storeme read: %d\n", childless->storeme);
    return sprintf(page, "%d\n", childless->storeme);
}

static ssize_t childless_showme_write(struct childless *childless, const char *page, size_t count)
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

    childless->storeme = tmp;

    return count;
}

static ssize_t childless_description_read(struct childless *childless, char *page)
{
    return sprintf(page, "Simplest possible subsystem in configfs\n");
}

//attribute under the childless. The dir corresponds to a kernel object, while the attribute under it corresponds a operation to it.
static struct childless_attribute childless_attr_showme = {
    .attr = {
        .ca_owner = THIS_MODULE,
        .ca_name = "showme",
        .ca_mode = S_IRUGO
    },
    .show = childless_showme_read,
};

static struct childless_attribute childless_attr_storeme = {
    .attr = {
        .ca_owner = THIS_MODULE,
        .ca_name = "storeme",
        .ca_mode = S_IRUGO | S_IWUSR
    },
    .show = childless_storeme_read,
    .store = childless_showme_write,
};

static struct childless_attribute childless_attr_description = {
    .attr = {
        .ca_owner = THIS_MODULE,
        .ca_name = "description",
        .ca_mode = S_IRUGO
    },
    .show = childless_description_read,
};

// configfs itself just goes here, all above encapsulation is just an OOP-way to deal with attribute of object childess. A good practice.
static struct configfs_attribute *childless_attrs[] = { //->4, KEY:configfs_attribute
    &childless_attr_showme.attr,
    &childless_attr_storeme.attr,
    &childless_attr_description.attr,
    NULL
};

static ssize_t childless_attr_show(struct config_item *item,
                    struct configfs_attribute *attr,
                    char *page)
{
    struct childless *childless = to_childless(item);
    struct childless_attribute *childless_attr =
            container_of(attr, struct childless_attribute, attr);
    ssize_t ret = 0;

    if (childless_attr->show)
        ret = childless_attr->show(childless, page);

    return ret;
}

static ssize_t childless_attr_store(struct config_item *item,
                    struct configfs_attribute *attr,
                    const char *page, size_t count)
{
    struct childless *childless = to_childless(item);
    struct childless_attribute *childless_attr =
            container_of(attr, struct childless_attribute, attr);
    ssize_t ret = -EINVAL;

    if (childless_attr->store)
        ret = childless_attr->store(childless, page, count);

    return ret;
}

static struct configfs_item_operations childless_item_ops = { //->4, KEY:configfs_item_operations
    .show_attribute = childless_attr_show,  //common operations for all items in current subsystem
    .store_attribute = childless_attr_store,
    //.release
    //.allow_link
    //.drop_link
};

static struct config_item_type childless_type = { //->3
    .ct_item_ops = &childless_item_ops, //operations for attribute
    .ct_attrs = childless_attrs,        //attributes to be operated
    .ct_owner = THIS_MODULE,
};

static struct childless childless_subsys = { //->2 .subsys is a configfs_group type. Each group belonging to a specific subsystem.
    .subsys = {
        .su_group = {       //KEY: config_group
            .cg_item = {    //KEY: config_item
                .ci_namebuf = "childless",
                .ci_type = &childless_type, //KEY:confi_item_type
            },
            //struct config_item cg_item,
            //struct list_head cg_children
            //struct configfs_subsystem *cg_subsys,     recursivly?
        },
    },
};


static struct configfs_subsystem *example_subsys[] = { //->1 top level, a sub systems array. 
    &childless_subsys.subsys, 
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
            printk(KERN_ERR "Error %d while registering subsystem %s\n",
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
