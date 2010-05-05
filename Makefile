#obj-m += hello-1.o
#obj-m += hello-2.o
#obj-m += hello-3.o
#obj-m += hello-4.o
#obj-m += hello-5.o
#obj-m += startstop.o
#obj-m += chardev.o
#startstop-objs := start.o stop.o

obj-m += xodev.o

export EXTRA_CFLAGS := -std=gnu99
xodev-objs := xomain.o xoai.o




all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	lsmod | grep xodev && rmmod xodev; insmod xodev.ko

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
