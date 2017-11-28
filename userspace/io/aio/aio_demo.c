#include <stdlib.h>
#include <stdio.h>
#include <libaio.h>
#include <sys/stat.h>
#include <fcntl.h>


#define AIO_BLKSIZE 512
#define AIO_MAXIO 64

int srcfd = -1;
int dstfd = -1;

static void write_done_cb(io_context_t ctx, struct iocb *iocb, long res, long res2)
{
    if (res2)
        printf("aio write error\n");

    if (res != iocb->u.c.nbytes) {
        printf("write missed bytes expect %d got %d\n", iocb->u.c.nbytes, res);
        exit(1);
    }

    free(iocb->u.c.buf);
    free(iocb);
}

static void read_done_cb(io_context_t ctx, struct iocb *iocb, long res, long res2)
{
    int iosize = iocb->u.c.nbytes;
    char *buf = (char *)iocb->u.c.buf;
    off_t offset = iocb->u.c.offset;
    char *wrbuf = NULL;

    if (res2)
        printf("aio read\n");

    if (res != iosize) {
        printf("read missed bytes expect %d got %d\n", iocb->u.c.nbytes, res);
        exit(1);
    }

    if (posix_memalign((void **)&wrbuf, getpagesize(), AIO_BLKSIZE) < 0) {
        printf("posix_memalign failed\n");
        exit (1);
    }

    io_prep_pwrite(iocb, dstfd, wrbuf, iosize, offset);
    io_set_callback(iocb, write_done_cb);
    if (res = io_submit(ctx, 1, &iocb))
        printf("io_submit write error\n");

    printf("submit %d write request\n", res);
}

int main(int args, void *argv[])
{
    char *mystr = "hello aio\n";
    char *content = (char *)calloc(strlen(mystr), sizeof(char));
    io_context_t myctx;
    int rc;
    char *buff = NULL;
    int offset = 0;
    int num, i;

    /* prepare files for rw */
    if (args < 3) {
        printf("wrong number of input arguments, format should be:\n");
        printf("<cmd> src_file_path dst_file_path\n"); 
        exit(1);
    }

    srcfd = open(argv[1], O_RDWR);
    if (srcfd < 0) {
        printf("invalid src input file: %s\n", argv[1]);
        exit(0);
    }
    write(srcfd, mystr, strlen(mystr));

    dstfd = open(argv[2], O_RDWR);
    if (dstfd < 0) {
        printf("invalid dst output file: %s\n", argv[2]);
        close(srcfd);
        exit(0);
    }

    /* prepare aio */
    io_queue_init(AIO_MAXIO, &myctx);
    struct iocb *io = (struct iocb *)malloc(sizeof(struct iocb));
    int err = posix_memalign((void **)&buff, getpagesize(), AIO_BLKSIZE);
    if (err < 0) {
        printf("posix align error!\n");
        exit(0);
    }

    if (NULL == io) {
        printf("io out of memory\n");
        exit(1);
    }
    io_prep_pread(io, srcfd, buff, AIO_BLKSIZE, offset);
    io_set_callback(io, read_done_cb);


    rc = io_submit(myctx, 1, &io);
    if (rc < 0)
        printf("io_submit read error\n");
    printf("submit %d read request\n", rc);


    /* io event capture */
    struct io_event events[AIO_MAXIO];
    io_callback_t cb;
    struct iocb *tmp_iocb;

    num = io_getevents(myctx, 1, AIO_MAXIO, events, NULL);
    printf("get %d io events done\n", num);
    for (i = 0; i < num; i++) {
        cb = (io_callback_t)events[i].data;
        tmp_iocb = events[i].obj;
        printf("events[%d]: data = %x, res = %d, res2 = %d\n", i, cb, events[i].res, events[i].res2);

        cb(myctx, tmp_iocb, events[i].res, events[i].res2);
    }

    close(srcfd);
    close(dstfd);
    io_destroy(myctx);

    return 0;
}
