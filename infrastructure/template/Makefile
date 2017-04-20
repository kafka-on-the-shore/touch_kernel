# Makefile
# Intendations have to be tabs, not spaces
UNAME := $(shell uname -r)
KERNEL310 := 3.10
KERNELVERSION := $(findstring $(KERNEL310),$(UNAME))

ifeq ($(KERNELVERSION),3.10)

obj-m :=wq_test.o

INCLUDE := -I/usr/include/asm/
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all::
	$(MAKE) -C $(KDIR) $(INCLUDE) SUBDIRS=$(PWD) modules
 
else

TARGET := wq_test
INCLUDE := -I/lib/modules/`uname -r`/build/include -I/usr/include/asm/
CFLAGS := -O2 -Wall -DMODULE -D__KERNEL__ -DLINUX
CC := gcc

all:: ${TARGET}.o


${TARGET}.o: ${TARGET}.c
	$(CC) $(CFLAGS) ${INCLUDE} -c ${TARGET}.c
 
endif

clean:
	rm -f *.ko *.o *.mod.o *.mod.c *.symvers *.order
