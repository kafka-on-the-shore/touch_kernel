#include <linux/module.h>
#include <linux/fs.h>
#include <linux/version.h>

#define SAMPLEFS_MAGIC 0x73616d70  /* "SAMP" */

static int sfs_fill_super(struct super_block * sb, void * data, int silent)
{
    return 0;
}

//int sfs_get_sb(struct file_system_type *fs_type,
//        int flags, const char *dev_name, void *data, struct vfsmount *mnt)
//{
//    return get_sb_nodev(fs_type, flags, data, sfs_fill_super, mnt);
//}

static struct file_system_type sfs_fs_type = {
    .owner = THIS_MODULE,
    .name = "samplefs",
//    .get_sb = sfs_get_sb,
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
