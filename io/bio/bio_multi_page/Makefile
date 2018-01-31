obj-m := bio_multi_page.o

bio_multi_page_module-objs := bio_multi_page.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

