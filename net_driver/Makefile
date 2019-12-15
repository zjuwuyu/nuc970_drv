KERN_DIR=/home/wuyu/nuc970bsp/kernel

all:
	make -C $(KERN_DIR) M=`pwd` modules
	cp -v net_drv.ko ../../rootfs
#	make -C app 
clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
#	make -C app clean
	rm -rf modules.order

obj-m += net_drv.o
