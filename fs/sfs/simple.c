#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/buffer_head.h>
#include <linux/slab.h>

#include "super.h"

static int simplefs_readdir(struct file *filp, void *dirent, filldir_t filldir)
{
    loff_t pos = filp->f_pos;
    struct inode *inode = filp->f_dentry->d_inode;
    struct super_block *sb = inode->i_sb;
    struct buffer_head *bh;
    struct simplefs_inode *sfs_inode;
    struct simplefs_dir_record *record;
    int i;

    printk(KERN_INFO "Inside dir: pos [%lld], inode number[%lu], sb magic[%lu]\n", \
        pos, inode->i_ino, sb->s_magic);

    sfs_inode = inode->i_private;

    if (unlikely(!S_ISDIR(sfs_inode->mode))) {
        printk(KERN_ERR "inode %llu is not a directory", sfs_inode->inode_no);
        return -ENOTDIR;
    }

    bh = (struct buffer_head *)sb_bread(sb, sfs_inode->data_block_number);

    record = (struct simplefs_dir_record *)bh->b_data;
    for (i = 0; i < sfs_inode->dir_children_count; i++) {
        printk(KERN_INFO "Got filename: %s\n", record->filename);
        filldir(dirent, record->filename, SIMPLEFS_FILENAME_MAXLEN, pos, record->inode_no, DT_UNKNOWN);

        pos += sizeof(struct simplefs_dir_record);
        record++;
    }
    return 1;
}

const struct file_operations simplefs_dir_operations = {
    .owner = THIS_MODULE,
    .readdir = simplefs_readdir,
};

struct dentry *simplefs_lookup(struct inode *parent_inode,
                    struct dentry *child_dentry, unsigned int flags)
{
    // lookup is used for dentry association
    printk(KERN_INFO "I'm lookup dentry now\n");
    return NULL;
}

static struct inode_operations simplefs_inode_ops = {
    .lookup = simplefs_lookup,
};

/* Return a simplefs inode specified by inode_no */
struct simplefs_inode * simplefs_get_inode(struct super_block *sb, uint64_t inode_no)
{
    struct simplefs_super_block *sfs_sb = sb->s_fs_info;
    struct simplefs_inode *sfs_inode = NULL;

    int i;
    struct buffer_head *bh;

    /* read inode once when mounting, while it's not a normal case:) */
    bh = (struct buffer_head *)sb_bread(sb, SIMPLEFS_INODESTORE_BLOCK_NUMBER);
    sfs_inode = (struct simplefs_inode *)bh->b_data;

    for (i = 0; i < sfs_sb->inodes_count; i++) {
        if (sfs_inode->inode_no == inode_no) {
            return sfs_inode;
        }
        sfs_inode++;
    }

    return NULL;

}

int simplefs_fill_super(struct super_block *sb, void *data, int silent)
{
    struct inode *root_inode;
    struct buffer_head *bh;
    struct simplefs_super_block *sb_disk;

    //fs infomation should be loaded from superblock as below:
    bh = (struct buffer_head *)sb_bread(sb, SIMPLEFS_SUPERBLOCK_BLOCK_NUMBER);

    sb_disk = (struct simplefs_super_block *)bh->b_data;

    printk(KERN_INFO "The magic number obtained in disk is: [%lld]\n", sb_disk->magic);
    if (unlikely(sb_disk->magic != SIMPLEFS_MAGIC)) {
        printk(KERN_ERR "Bad magic, The fs that you try to mount is not crrectly format of type simplefs");
        return -EPERM;
    } else {
        sb->s_magic = SIMPLEFS_MAGIC;
    }

    if (unlikely(sb_disk->block_size != SIMPLEFS_DEFAULT_BLOCK_SIZE)) {
        printk(KERN_ERR "Wrong block size when formated");
        return -EPERM;
    }

    sb->s_fs_info = sb_disk;
    root_inode = new_inode(sb);
    root_inode->i_ino = SIMPLEFS_ROOTDIR_INODE_NUMBER;
    inode_init_owner(root_inode, NULL, S_IFDIR);
    root_inode->i_sb = sb;
    root_inode->i_op = &simplefs_inode_ops;
    root_inode->i_fop = &simplefs_dir_operations; //2->3
    root_inode->i_atime = root_inode->i_mtime = root_inode->i_ctime = CURRENT_TIME;
    root_inode->i_private = simplefs_get_inode(sb, SIMPLEFS_ROOTDIR_INODE_NUMBER);
    sb->s_root = d_make_root(root_inode);
    if (!sb->s_root)
        return -ENOMEM;

    return 0;
}

static void simplefs_kill_superblock(struct super_block *s)
{
    printk(KERN_INFO "simplefs superblock is destroyed, Unmount succesfully\n");
    return;
}

static struct dentry *simplefs_mount(struct file_system_type *fs_type,
                        int flags, const char * dev_name, void *data)
{
    struct dentry *ret;
    
    ret = mount_bdev(fs_type, flags, dev_name, data, simplefs_fill_super); //1->2

    if (unlikely(IS_ERR(ret)))
        printk(KERN_ERR "Error mounting simplefs");
    else
        printk(KERN_INFO "Simple fs is mounted succesfully on [%s]\n", dev_name);

    return ret;
}

struct file_system_type simplefs_fs_type = {
    .owner = THIS_MODULE,
    .name = "simplefs",
    .mount = simplefs_mount,
    .kill_sb = simplefs_kill_superblock,
};

static int __init simplefs_init(void)
{
    int ret;
    printk(KERN_ALERT "hello world");

    ret = register_filesystem(&simplefs_fs_type); //0->1
    if (likely(ret == 0))
        printk(KERN_INFO "Sucessfully register simplefs\n");
    else
        printk(KERN_ERR "Failed to register simplefs, Error: [%d]\n", ret);


    return ret;
}

static void __exit simplefs_exit(void)
{
    int ret;
    printk(KERN_ALERT "good bye");

    ret = unregister_filesystem(&simplefs_fs_type);
    if (likely(ret == 0))
        printk(KERN_INFO "Sucessfully unregister simplefs\n");
    else
        printk(KERN_ERR "Failed to unregister simplefs, Error: [%d]\n", ret);
}

module_init(simplefs_init);
module_exit(simplefs_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rock");
