#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/bio.h>
#include <linux/configfs.h>
#include <linux/fs.h>
#include <linux/file.h>

#include <linux/delay.h>


struct kafka {
    struct configfs_subsystem subsys;
    struct block_device *bdev;
    char bdev_name[BDEVNAME_SIZE];

    u64 blkno;
    u64 offset;
    size_t length;
    struct page *raw_data; 
    char data[0];
};

/* attributes  has different operations,
 * so we need to create one more layer to hidden this difference */
struct kafka_attribute {
    struct configfs_attribute attr;
    ssize_t (*show)(struct kafka *, char *);
    ssize_t (*store)(struct kafka *, const char *, size_t);
    void (*release)(struct kafka *);
};

static inline struct kafka *to_kafka(struct config_item *item)
{
    return item ?
        container_of(to_configfs_subsystem(to_config_group(item)), struct kafka, subsys) :
        NULL;
}

static ssize_t kafka_attr_show(struct config_item *item,
                    struct configfs_attribute *attr,
                    char *page)
{
    struct kafka *kafka = to_kafka(item);
    struct kafka_attribute *kafka_attr =
        container_of(attr, struct kafka_attribute, attr);
    ssize_t ret = 0;

    if (kafka_attr->show)
        ret = kafka_attr->show(kafka, page);

    return ret;
}

static ssize_t kafka_attr_store(struct config_item *item,
                    struct configfs_attribute *attr,
                    const char *page, size_t count)
{
    struct kafka *kafka = to_kafka(item);
    struct kafka_attribute *kafka_attr =
        container_of(attr, struct kafka_attribute, attr);
    ssize_t ret = -EINVAL;

    if (kafka_attr->store)
        ret = kafka_attr->store(kafka, page, count);

    return ret;
}

static void kafka_release(struct config_item *item)
{
    struct kafka *kafka = to_kafka(item);
    if (kafka->data)
        kfree(kafka->data);
}

/* attribute: kafka/postion */
static ssize_t kafka_postion_read(struct kafka *kafka, char *page)
{
    ssize_t pos;
    pos = sprintf(page, "%lld@%lld@%ld\n", 
                    (long long int)kafka->blkno,
                    (long long int)kafka->offset,
                    (long int)kafka->length);

    return pos;
}

static ssize_t kafka_postion_write(struct kafka *kafka, const char *page, size_t count)
{    
    if (sscanf(page, "%lld@%lld@%ld",
                (u64 *)(&kafka->blkno),
                (u64 *)(&kafka->offset),
                (size_t *)(&kafka->length)) != 3)
        return -EINVAL;
    if (kafka->offset % 512 != 0 || kafka->length % 512 != 0) {
        printk(KERN_ERR "offset and length must be 512-byte align.\n");
        return EINVAL;
    }
    
    return count;
}

static struct kafka_attribute kafka_attr_postion = {
    .attr = { .ca_owner = THIS_MODULE, .ca_name = "postion", .ca_mode = S_IRUGO | S_IWUSR },
    .show = kafka_postion_read,
    .store = kafka_postion_write,
};

/* attribute: kafka/data */
static void kafka_read_complete(struct bio *bio, int err)
{
    if (err) {
        printk(KERN_ERR "Lock area IO Error %d\n", err);
    }

    complete(bio->bi_private);
    bio_put(bio);
}

static int kafka_block_io(struct kafka *kafka, int do_write)
{
    int ret;
    unsigned int vec_len, vec_start;
    struct bio *bio; 
    struct completion event;
    char buff[BDEVNAME_SIZE];

    init_completion(&event);
    bio = bio_alloc(GFP_KERNEL, 1);
    bio->bi_bdev = kafka->bdev;
    bio->bi_sector = (kafka->blkno << (12 - 9)) + kafka->offset/512;  //4K = 2**12, and sector is 2**9
    bio->bi_private = &event;
    bio->bi_end_io = kafka_read_complete;

    printk(KERN_ERR "bi_bdev: %s, bi_sector: %d\n", bdevname(bio->bi_bdev, buff), (int)bio->bi_sector);
    vec_len = kafka->length;
    vec_start = kafka->offset;
    bio_add_page(bio, kafka->raw_data, vec_len, 0);

    do_write ? submit_bio(WRITE|REQ_SYNC, bio) : submit_bio(READ|REQ_SYNC, bio);
    wait_for_completion(&event);

    ret = test_bit(BIO_UPTODATE, &bio->bi_flags);
    printk(KERN_ERR "test_bit result: %d\n", ret);

    return ret;
}

static ssize_t kafka_data_read(struct kafka *kafka, char *page)
{
    ssize_t pos;
    char *raw;

    pos = 0;
    kafka_block_io(kafka, 0);
    raw = page_address(kafka->raw_data);
    memcpy(page, raw, kafka->length);

    pos += kafka->length;

    return pos;
}

static ssize_t kafka_data_write(struct kafka *kafka, const char *page, size_t count)
{
    char *raw;

    BUG_ON(count > 4096);

    raw = page_address(kafka->raw_data);
    memcpy(raw, page, kafka->length);
    kafka_block_io(kafka, 1);

    return count;
}

static struct kafka_attribute kafka_attr_data= {
    .attr = { .ca_owner = THIS_MODULE, .ca_name = "data", .ca_mode = S_IRUGO | S_IWUSR },
    .show = kafka_data_read,
    .store = kafka_data_write,
};

/* attribute: kafka/bdev */
static ssize_t kafka_bdev_write(struct kafka *kafka, const char *page, size_t count)
{
    int ret;
    long fd;
    struct fd f;
    char *p = (char *)page;

    fd = simple_strtoul(p, &p, 10);
    if (!p || ( *p && (*p != '\n'))) {
        printk("Wrong input of bdev.\n");
        goto out;
    }

    f = fdget(fd);
    if (f.file == NULL)
        goto out;
    
    kafka->bdev = I_BDEV(f.file->f_mapping->host);
    ret = blkdev_get(kafka->bdev, FMODE_WRITE | FMODE_READ, NULL);
    if (ret) {
        printk(KERN_ERR "Failed to excute blkdev_get.\n");
        kafka->bdev = NULL;
        goto out;
    }

    /* TODO: need to be release */
    bdevname(kafka->bdev, kafka->bdev_name);
    printk(KERN_ERR "bdev_name: %s.\n", kafka->bdev_name);

out:
    fdput(f);
    return count;
}

static ssize_t kafka_bdev_read(struct kafka *kafka, char *page)
{
    ssize_t pos;
    pos = sprintf(page, "%s\n", kafka->bdev_name);

    return pos;
}

static struct kafka_attribute kafka_attr_bdev= {
    .attr = { .ca_owner = THIS_MODULE, .ca_name = "bdev", .ca_mode = S_IRUGO | S_IWUSR },
    .show = kafka_bdev_read,
    .store = kafka_bdev_write,
};

/* attribute array of kafka item */
static struct configfs_attribute *kafka_attrs[] = {
    &kafka_attr_postion.attr,
    &kafka_attr_data.attr,
    &kafka_attr_bdev.attr,
    NULL,
};

static struct configfs_item_operations kafka_item_ops = {
    .release            = kafka_release,
    .show_attribute     = kafka_attr_show,
    .store_attribute    = kafka_attr_store,
};

static struct config_item_type kafka_type = {
    .ct_item_ops = &kafka_item_ops,
    .ct_attrs = kafka_attrs,
    .ct_owner = THIS_MODULE,
};

static struct kafka configfs_kafka = {
    .subsys = {
        .su_group = {
            .cg_item = {
                .ci_namebuf = "kafka",
                .ci_type = &kafka_type,
            },
        },
    },
};

int kafka_configfs_init(struct configfs_subsystem *subsys)
{
    int ret;

    config_group_init(&subsys->su_group);
    mutex_init(&subsys->su_mutex);
    ret = configfs_register_subsystem(subsys);

    return ret;
}

static void kafka_configfs_exit(struct configfs_subsystem *subsys)
{
    configfs_unregister_subsystem(subsys);
}


static int __init configfs_bio_module_init(void)
{
    int ret;
    struct kafka *la = &configfs_kafka;

    ret = kafka_configfs_init(&la->subsys);
    if (ret) {
        printk(KERN_ERR "Failed to register kafka configfs  module\n");
        return ret;
    }

 
    printk(KERN_ERR "allocate raw_data page now\n");
    la->raw_data = alloc_page(GFP_KERNEL);
    if (!la->raw_data) {
        kafka_configfs_exit(&la->subsys);
        return -ENOMEM;
    } 

    la->blkno = -1;

    return 0;
}

static void __exit configfs_bio_module_exit(void)
{
    struct kafka *la = &configfs_kafka;
    if (la->bdev) {
        blkdev_put(la->bdev, FMODE_READ | FMODE_WRITE);
        la->bdev = NULL;
    }
    kafka_configfs_exit(&la->subsys);
}

module_init(configfs_bio_module_init);
module_exit(configfs_bio_module_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Inspur");
