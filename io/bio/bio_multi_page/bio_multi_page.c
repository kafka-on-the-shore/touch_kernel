#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/bio.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/delay.h>

static char *my_dev_path = "/dev/sdc";
module_param(my_dev_path, charp, S_IRUGO);

#define MY_PAGES 16

struct my_data {
    __u64 a;
    __u32 b;
    __u8  c[4];
};

struct disk_io {
    struct page **raw_pages;
    struct my_data **data;
};

static struct disk_io my_disk_io;

static int setup(void)
{
    int i;
    struct page *tmp;

    my_disk_io.raw_pages = kmalloc(sizeof(struct page *)*MY_PAGES, GFP_KERNEL);
    if (!my_disk_io.raw_pages) {
        printk(KERN_ERR "failed to allocate page pointer\n");
        return -ENOMEM;
    }

    my_disk_io.data = kmalloc(sizeof(struct my_data *)*MY_PAGES, GFP_KERNEL);
    if (!my_disk_io.data) {
        printk(KERN_ERR "failed to allocate data pointer\n");
        return -ENOMEM;
    }

    for (i = 0; i < MY_PAGES; i++) {
        tmp = alloc_page(GFP_KERNEL);
        if (!tmp) {
            printk(KERN_ERR "failed to allocate pages\n");
            return -ENOMEM;
        }
        my_disk_io.raw_pages[i] = tmp;
        my_disk_io.data[i] = (struct my_data *)page_address(tmp);
        memset(my_disk_io.data[i], 0, 4096);
    }

    return 0;
}

static void destroy(void)
{
    int i;
    struct page *pp;

    for (i = 0; i < MY_PAGES; i++) {
        pp = my_disk_io.raw_pages[i];
        if (pp)
            __free_page(pp);
    }

    kfree(my_disk_io.raw_pages);
    kfree(my_disk_io.data);
}

static void kafka_read_complete(struct bio *bio, int err)
{
    if (err) {
        printk(KERN_ERR KERN_ERR "Lock area IO Error %d\n", err);
    }

    complete(bio->bi_private);
    bio_put(bio);
}

static int kafka_block_io(struct block_device *bdev, int do_write)
{
    int ret;
    unsigned int vec_len, vec_start;
    struct bio *bio; 
    struct completion event;
    int i;

    init_completion(&event);
    /* trap: only reserve 8 bio vectors, so you will find that redunant pages
     * will be dropped due to lack of vectors, although there is no warning
     * */
    bio = bio_alloc(GFP_KERNEL, 4);
    bio->bi_bdev = bdev;
    bio->bi_sector = 100 << 3;
    bio->bi_private = &event;
    bio->bi_end_io = kafka_read_complete;

    vec_len = 512;
    vec_start = 0;
    for (i = 0; i < 8; i++) {
        /* trap: the length and start increasing are intended, so we can observe
         * how these parameters works. We will see that the content will be
         * written to offset 0x200, 0x600, 0xc00, 0x1400... While the content will
         * drop one charactor each time due to we increase one byte of the
         * start address in each loop.
         * Just for demo purpose:)
         */
        bio_add_page(bio, my_disk_io.raw_pages[i], vec_len+512*i, vec_start+i);
    }

    do_write ? submit_bio(WRITE|REQ_SYNC, bio) : submit_bio(READ|REQ_SYNC, bio);
    wait_for_completion(&event);

    ret = test_bit(BIO_UPTODATE, &bio->bi_flags);
    printk(KERN_ERR "test_bit result: %d\n", ret);

    return ret;
}


static int __init bio_multi_page_module_init(void)
{
    struct block_device *bdev = NULL;
    struct my_data *pdata;
    int i;

    printk(KERN_INFO "bio_multi_page start ...");
    setup();

    bdev = blkdev_get_by_path(my_dev_path, FMODE_WRITE | FMODE_READ, NULL);
    if (!bdev) {
        printk(KERN_ERR KERN_ERR "Failed to excute blkdev_get.\n");
        return -EINVAL;
    }

    for (i = 0; i < MY_PAGES; i++) {
        pdata = my_disk_io.data[i];
        pdata->a = 0x123456789abcdef + i;
        pdata->b = 0xdeadbeef;
        sprintf(pdata->c, "H%d", i);
    }

    printk(KERN_INFO "start to block io...");
    kafka_block_io(bdev, WRITE);

    blkdev_put(bdev, FMODE_WRITE | FMODE_READ);

    return 0;
}

static void __exit bio_multi_page_module_exit(void)
{
    printk(KERN_ERR "exit module\n");
    destroy();
}

module_init(bio_multi_page_module_init);
module_exit(bio_multi_page_module_exit);
MODULE_LICENSE("GPL");
