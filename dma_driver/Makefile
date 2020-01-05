KERN_DIR=/home/wuyu/nuc970bsp/kernel

all:
	make -C $(KERN_DIR) M=`pwd` modules
	make -C app 
	cp dma_drv.ko  ../../rootfs
	cp app/dma_drv_test ../../rootfs
clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	make -C app clean
	rm -rf modules.order

obj-m += dma_drv.o
