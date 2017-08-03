#define SIMPLEFS_MAGIC 0x10032013
#define SIMPLEFS_DEFAULT_BLOCK_SIZE 4096
#define SIMPLEFS_FILENAME_MAXLEN 255

/* disk block where super block is stored */
#define SIMPLEFS_SUPERBLOCK_BLOCK_NUMBER 0

/* disk block where inodes are stored */
#define SIMPLEFS_INODESTORE_BLOCK_NUMBER 1

/* disk block where root inode is stored */
#define SIMPLEFS_ROOTDIR_INODE_NUMBER 1

/* disk block of data of root dir itself stored */
#define SIMPLEFS_ROOTDIR_DATABLOCK_NUMBER 2


struct simplefs_inode {
    mode_t mode;
    uint64_t inode_no;
    uint64_t data_block_number;

    union {
        uint64_t file_size;
        uint64_t dir_children_count;
    };
};

struct simplefs_super_block {
    uint64_t version;
    uint64_t magic;
    uint64_t block_size;
    uint64_t inodes_count;
    uint64_t free_blocks;

    struct simplefs_inode root_inode;

    char padding[SIMPLEFS_DEFAULT_BLOCK_SIZE - 5*sizeof(uint32_t)]; 
};

struct simplefs_dir_record {
    char filename[SIMPLEFS_FILENAME_MAXLEN];
    uint64_t inode_no;
};

struct simplefs_dir_contents {
    uint32_t children_count;
    struct simplefs_dir_record records[];
};
