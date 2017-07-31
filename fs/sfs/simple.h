#define SIMPLEFS_MAGIC 0x10032013
#define SIMPLEFS_DEFAULT_BLOCK_SIZE 4096

struct simplefs_super_block {
    uint32_t version;
    uint32_t magic;
    uint32_t block_size;
    uint32_t free_blocks;

    char padding[SIMPLEFS_DEFAULT_BLOCK_SIZE - 4*sizeof(uint32_t)];
};
