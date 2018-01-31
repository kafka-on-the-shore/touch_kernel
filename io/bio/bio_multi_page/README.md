# bio example codes 2

> Demostration how to use bio with muliple bio vectors

usage:
1. make and insert bio_multi_page module `insmod bio_multi_page.ko`.
2. rmmove module `rmmod bio_multi_page`
3. dump out the contents:
   `dd if=/dev/sdc of=dump bs=4K count=16 skip=100 iflag=direct`
4. check if the content was modified with `vi` and `xxd` command.
