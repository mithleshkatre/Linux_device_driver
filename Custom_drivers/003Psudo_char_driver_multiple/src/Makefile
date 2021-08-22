obj-m := pcd_n.o

ARCH = arm

CROSS_COMPILE = arm-linux-gnueabihf-

KER_DIR = /home/mithlesh/BBB_Workspace/ldd/sources/linux-4.14/

Host_KER_DIR = /lib/modules/$(shell uname -r)/build/

all:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KER_DIR) M=$(PWD) modules

clean:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KER_DIR) M=$(PWD) clean

help:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KER_DIR) M=$(PWD) help

host:
	make -C $(Host_KER_DIR) M=$(PWD) modules

















