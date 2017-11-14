#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#define NAME_MAX 156
int main()
{
    int fd, bdev_fd,  count, ret;
    char num_buf[NAME_MAX] = {0};
    char temp;

    fd = open("/dev/sdb", O_RDWR);
    if (fd < 0) {
        printf("Open /dev/sdb failed, %d.\n", fd);
        goto out1;
    }

    count = snprintf(num_buf, NAME_MAX -1, "%d", fd);
    if (count < 0) {
        printf("failed to write num_buf\n");
        goto out2;
    }

    bdev_fd = open("/sys/kernel/config/kafka/bdev", O_WRONLY);
    if (bdev_fd < 0) {
        printf("write bdev failed, %d\n", bdev_fd);
        goto out3;
    }
    ret = write(bdev_fd, num_buf, count + 1);
    if (ret - 1 != count) {
        printf("failed to write %d : %s to bdev, because count %d != ret %d", fd, num_buf, count, ret);
    }
    getchar();

out3:
    printf("in out3\n");
    close(bdev_fd);
    getchar();
out2:
    printf("in out2\n");
    temp = close(fd);
    getchar();
out1:
    return 1;
}
