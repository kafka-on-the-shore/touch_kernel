#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>

#include "simple.h"

int main(int argc, char *argv[])
{
    int fd;
    ssize_t ret;
    struct simplefs_super_block sb;

    if (argc != 2) {
        printf("Usage: mkfs-simplefs <device>\n");
        return -1;
    }

    fd = open(argv[1], O_RDWR);
    if (fd == -1) {
        perror("Error opening device");
        return -1;
    }

    sb.version = 1;
    sb.magic = SIMPLEFS_MAGIC;
    sb.block_size = SIMPLEFS_DEFAULT_BLOCK_SIZE;
    sb.free_blocks = ~0;

    sb.root_inode.mode = S_IFDIR;
    sb.root_inode.inode_no = SIMPLEFS_ROOT_INODE_NUMBER;
    sb.root_inode.data_block_number = SIMPLEFS_ROOTDIR_DATABLOCK_NUMBER;
    sb.root_inode.dir_children_count = 0;

    ret = write(fd, (char *)&sb, sizeof(sb));

    if (ret != SIMPLEFS_DEFAULT_BLOCK_SIZE)
        printf("bytes written [%d] is not equal to block size [%d]\n", (int)ret, SIMPLEFS_DEFAULT_BLOCK_SIZE);
    else
        printf("Super block written sucesfully\n");

    close(fd);

    return 0;
}
