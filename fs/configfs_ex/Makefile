obj-m := configfs_childless.o configfs_simple_children.o
configfs_ex-objs := configfs_childless.o configfs_simple_children.o


all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules


clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

