ifneq ($(KERNELRELEASE),)

mymodule-objs := super.o
obj-m := super.o

else

PWD := $(shell pwd)
KDIR :=/lib/modules/`uname -r`/build

all:
	$(MAKE) -C $(KDIR) M=$(PWD)

clean:
	rm -rf *.mod.c *.ko *.o *.order *.symvers .super.* .built* .tmp*


endif
