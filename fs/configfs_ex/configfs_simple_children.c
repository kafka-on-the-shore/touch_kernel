
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>

#include <linux/configfs.h>

/*
 * simple children example: fun
 *
 * We can create multipath copies of fun item under foo folder by mkdir, or remove it by rmdir.
 *
 * Reference: https://lwn.net/Articles/149007
 */

struct fun {
    struct config_item item;
    int counter;
};

static inline struct fun *to_fun(struct config_item *item)
{
    return item ? container_of(item, struct fun, item) : NULL;
}

/* attrs: */
static struct configfs_attribute fun_attr_counter = {
    .ca_owner   = THIS_MODULE,
    .ca_name    = "counter",
    .ca_mode    = S_IRUGO | S_IWUSR,
};

static struct configfs_attribute *fun_attrs[] = {
    &fun_attr_counter,
    NULL,
};


/* item ops: */
static ssize_t fun_attr_show(struct config_item *item,
                    struct configfs_attribute *attr,
                    char *page)
{
    ssize_t count = 0;
    struct fun *fun = to_fun(item);

    count = sprintf(page, "%d\n", fun->counter);

    return count;
}

static ssize_t fun_attr_store(struct config_item *item,
                    struct configfs_attribute *attr,
                    const char *page, size_t count)
{
    long tmp;
    char *p = (char *)page;
    struct fun *fun = to_fun(item);

    printk(KERN_INFO "page content: %s\n", page);
    tmp = simple_strtoul(p, &p, 10);
    printk(KERN_INFO "page content after convert: %ld\n", tmp);
    if (!p || (*p && (*p != '\n')))
        return -EINVAL;

    if ( tmp > INT_MAX)
        return -ERANGE;

    fun->counter = tmp;

    return count;
}

static void fun_release(struct config_item *item)
{
    kfree(to_fun(item));
}

static struct configfs_item_operations fun_item_ops = {
    .release            = fun_release,
    .show_attribute     = fun_attr_show,
    .store_attribute    = fun_attr_store,
};

static struct config_item_type fun_type = {
    .ct_item_ops    = &fun_item_ops,
    .ct_attrs       = fun_attrs,
    .ct_owner       = THIS_MODULE,
};

static struct config_item *fun_make_item(struct config_group *group, const char *name)
{
    struct fun *fun;

    fun = kzalloc(sizeof(struct fun), GFP_KERNEL);
    if (!fun)
        return NULL;

    config_item_init_type_name(&fun->item, name, &fun_type);
    fun->counter = 0;

    return &fun->item;
}

static void fun_drop_item(struct config_group *group, struct config_item *item)
{
    struct fun *myfun = to_fun(item);
    printk(KERN_ERR "drop item %s\n", config_item_name(item));
    if (myfun)
        kfree(myfun);
    config_item_put(item);
}

/* top attr: description*/
static struct configfs_attribute fun_top_attr_description = {
    .ca_owner   = THIS_MODULE,
    .ca_name    = "description",
    .ca_mode    = S_IRUGO,
};

/* why another attrs? */
static struct configfs_attribute *fun_top_attrs[] = {
    &fun_top_attr_description,
    NULL,
};

static ssize_t fun_top_attr_show(struct config_item *item,
                    struct configfs_attribute *attr,
                    char *page)
{
    return sprintf(page, "I'm descripiton attribute\n");
}

static struct configfs_item_operations fun_top_item_ops = {
    .show_attribute = fun_top_attr_show,
};


static struct configfs_group_operations fun_group_ops = {
    .make_item = fun_make_item,
    .drop_item = fun_drop_item,
};

static struct config_item_type fun_top_type = {
    .ct_item_ops    = &fun_top_item_ops,
    .ct_group_ops   = &fun_group_ops,  //key difference with this childless example: we support to create new item.
    .ct_attrs       = fun_top_attrs,
};


static struct configfs_subsystem fun_subsys = {
    .su_group = {       //KEY: config_group
        .cg_item = {    //KEY: config_item
            .ci_namebuf = "fun",
            .ci_type = &fun_top_type, //KEY:confi_item_type
        },
    },
};


static struct configfs_subsystem *example_subsys[] = { //->1 top level, a sub systems array. 
    &fun_subsys,
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
