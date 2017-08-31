
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>

#include <linux/configfs.h>

/*
 * foo example
 *
 * Can not create any config_items, it just has attribute
 *
 * Reference: https://lwn.net/Articles/149007
 */

struct foo {
   struct configfs_subsystem subsys; //->
   int showme;
   int storeme;
};

struct child {
    struct config_item item;
    int storeme;
};


// An OOP desgin for deal with each attribute, all items of this obj support show/store methods
struct foo_attribute {
    struct configfs_attribute attr; //->
    ssize_t (*show)(struct foo *, char*);
    ssize_t (*store)(struct foo *, const char *, size_t);
};

//cause foo include subsys, so foo itself is teared from a config_item
static inline struct foo *to_foo(struct config_item *item)
{
    return item ? container_of(to_configfs_subsystem(to_config_group(item)), struct foo, subsys) : NULL;
}

static inline struct child *to_child(struct config_item *item)
{
    return item ? container_of(item, struct child, item) : NULL;
}

static struct configfs_attribute child_attr_storeme = {
    .ca_owner = THIS_MODULE,
    .ca_name = "storeme",
    .ca_mode = S_IRUGO | S_IWUSR,
};

static struct configfs_attribute * child_attrs[] = {
    &child_attr_storeme,
    NULL,
};

static ssize_t child_attr_show(struct config_item *item,
                    struct configfs_attribute * attr,
                    char *page)
{
    ssize_t count;
    struct child *child = to_child(item);

    printk(KERN_INFO "child show: %d\n", child->storeme);
    count = sprintf(page, "%d\n", child->storeme);
    return count;
}

static ssize_t child_attr_store(struct config_item *item,
                    struct configfs_attribute * attr,
                    const char *page, size_t count)
{
    struct child *child = to_child(item);
    unsigned long tmp;
    char *p = (char *)page;

    tmp = simple_strtoul(p, &p, 10);
    printk(KERN_INFO "page content after convert: %ld\n", tmp);
    if (!p || (*p && (*p != '\n')))
        return -EINVAL;

    if ( tmp > INT_MAX)
        return -ERANGE;

    child->storeme = tmp;

    return count;
}

static void child_release(struct config_item *item)
{
    kfree(to_child(item));
}

static struct configfs_item_operations child_item_ops = {
    .release = child_release,
    .show_attribute = child_attr_show,
    .store_attribute = child_attr_store,
};

static struct config_item_type child_type = {
    .ct_item_ops = &child_item_ops,
    .ct_attrs = child_attrs,
    .ct_owner = THIS_MODULE,
};

static struct config_item *child_make_item(struct config_group *group, const char *name)
{
    struct child *child = kzalloc(sizeof(struct child), GFP_KERNEL);
    if (!child)
        return NULL;

    config_item_init_type_name(&child->item, name, &child_type);

    child->storeme = 0;

    return &child->item;
}

void child_drop_item(struct config_group *group, struct config_item *item)
{
    struct child *child = to_child(item);
    if (child)
        kfree(child);
    config_item_put(item);
}

static struct configfs_group_operations child_group_ops = {
    .make_item = child_make_item,
    .drop_item = child_drop_item,
};

static ssize_t foo_showme_read(struct foo *foo, char *page)
{
    ssize_t pos;

    pos = sprintf(page, "%d\n", foo->showme);
    foo->showme++;
    printk(KERN_INFO "showme read: %d\n", foo->showme);

    return pos;
}

static ssize_t foo_storeme_read(struct foo *foo, char *page)
{
    printk(KERN_INFO "storeme read: %d\n", foo->storeme);
    return sprintf(page, "%d\n", foo->storeme);
}

static ssize_t foo_showme_write(struct foo *foo, const char *page, size_t count)
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

    foo->storeme = tmp;

    return count;
}

static ssize_t foo_description_read(struct foo *foo, char *page)
{
    return sprintf(page, "Simplest possible subsystem in configfs\n");
}

//attribute under the foo. The dir corresponds to a kernel object, while the attribute under it corresponds a operation to it.
static struct foo_attribute foo_attr_showme = {
    .attr = {
        .ca_owner = THIS_MODULE,
        .ca_name = "showme",
        .ca_mode = S_IRUGO
    },
    .show = foo_showme_read,
};

static struct foo_attribute foo_attr_storeme = {
    .attr = {
        .ca_owner = THIS_MODULE,
        .ca_name = "storeme",
        .ca_mode = S_IRUGO | S_IWUSR
    },
    .show = foo_storeme_read,
    .store = foo_showme_write,
};

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
    &foo_attr_showme.attr,
    &foo_attr_storeme.attr,
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
    //.release,
    //.allow_link
    //.drop_link
};

static struct config_item_type foo_type = { //->3
    .ct_item_ops = &foo_item_ops, //operations for attribute
    .ct_group_ops = &child_group_ops,
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
            //struct config_item cg_item,
            //struct list_head cg_children
            //struct configfs_subsystem *cg_subsys,     recursivly?
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
