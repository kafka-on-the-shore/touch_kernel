# bio example codes

> Demostration how to use configfs and bio

usage:
1. make and insert configfs_bio module.
2. compiling bio_test.c: `gcc test.c -o test`
3. run *test* so it will set the block device in configfs.
4. set access postion:
   ```
   echo 10@1024@512 > /sys/kernel/config/kafka/postion
   ```
   the postion parameter format: `blkno@offset@length`
   > offset and length must be aligned with 512 byte.
5. then we can read/write the data by `cat data` or
   echo "hello world" > data under /sys/kernel/config/kafka
 

