obj-m += xodev.o

EXTRA_CFLAGS := -std=gnu99
xodev-objs := xomain.o xoai.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

insert:
	lsmod | grep xodev && rmmod xodev; insmod xodev.ko

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
