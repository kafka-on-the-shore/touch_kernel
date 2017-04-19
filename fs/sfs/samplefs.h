#define SAMPLEFS_ROOT_I 2

struct samplefs_sb_info {
    unsigned int rsize;
    unsigned int wsize;
    int mnt_flags;
    struct nls_table *local_nls;
};

static inline struct samplefs_sb_info *
SFS_SB(struct super_block *sb)
{
    return sb->s_fs_info;
}
