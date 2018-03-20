EXTRA_CFLAGS := -g
obj-m += seqfile_ex.o

default:
	make -C /lib/modules/$(shell uname -r)/build/ SUBDIRS=$(shell pwd) modules 

clean:
	make -C /lib/modules/$(shell uname -r)/build/ SUBDIRS=$(shell pwd) clean
