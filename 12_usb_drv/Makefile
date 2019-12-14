KERN_DIR=/home/wuyu/nuc970bsp/kernel

all:
	make -C $(KERN_DIR) M=`pwd` modules
#	make -C app 
clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
#	make -C app clean
	rm -rf modules.order

obj-m += 12_usb_drv.o
