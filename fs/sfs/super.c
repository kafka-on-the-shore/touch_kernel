#include <linux/module.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <linux/pagemap.h>
#include "samplefs.h"

#define SAMPLEFS_MAGIC 0x73616d70  /* "SAMP" */

static void
sfs_put_super(struct super_block *sb)
{
    struct samplefs_sb_info *sfs_sb;

    sfs_sb = SFS_SB(sb);
    if (sfs_sb == NULL) {
        // when empty superblock info passed to unmount
        return;
    }

    unload_nls(sfs_sb->local_nls);

    kfree(sfs_sb);
    return
}

struct super_operations sample_super_ops = {
    .statfs = simple_statfs,
    .drop_inode = generic_delete_inode,
    .put_super = samplefs_put_super
}

struct void
sfs_parse_mount_options(char *options, struct samplefs_sb_info *sfs_sb)
{
    char *value;
    char *data;
    int size;

    if (!options)
        return;

    while ((data = strsep(&options, ",")) != NULL) {
        if (!*data)
            continue;

        if ((value = strchr(data, "=")) != NULL)
            *value++ = '\0';

        if (strnicmp(data, "rsize", 5) == 0) {
            if (value && *value) {
                size = simple_stroul(value, &value , 0);
                if (size > 0)
                    sfs_sb->rsize = size;
            }
        } else if (strnicmp(data, "wsize", 5) == 0) {
            if (value && *value) {
                size = simple_stroul(value, &value , 0);
                if (size > 0)
                    sfs_sb->wsize = size;
            }
        }
    }
}

static int sfs_fill_super(struct super_block * sb, void * data, int silent)
{
    struct inode *inode;
    struct samplefs_sb_info *sfs_sb;

    sb->s_maxbytes = MAX_LFS_FILESIZE;
    sb->s_blocksize = PAGE_CACHE_SIZE;
    sb->s_blocksize_bits = PAGE_CACHE_SHIFT;
    sb->s_magic = SAMPLEFS_MAGIC;
    sb->s_op = &sample_super_ops;
    return 0;
}

static struct file_system_type sfs_fs_type = {
    .owner = THIS_MODULE,
    .name = "samplefs",
    .kill_sb = kill_anon_super,
};

static int __init init_samplefs_fs(void)
{
    printk(KERN_WARNING "register sample fs now...\n");
    return register_filesystem(&sfs_fs_type);
}

static void __exit exit_samplefs_fs(void)
{
    printk(KERN_WARNING "unregister sample fs now...\n");
    unregister_filesystem(&sfs_fs_type);
}

module_init(init_samplefs_fs);
module_exit(exit_samplefs_fs);
MODULE_LICENSE("GPL");
