OBJS := start.o lowlevel_init.o led.o serial.o nand.o main.o setup_tags.o
CLFAGS = -fno-builtin -ffunction-sections -O0

test: $(OBJS)
	arm-linux-ld.bfd -Ttest.lds   -o $@ $^ 
	arm-linux-objdump -D $@ > $@.dis
	arm-linux-objcopy -O binary $@ $@.bin

%.o: %.S
	arm-linux-gcc $(CLFAGS) -c $^

%.o: %.c
	arm-linux-gcc $(CLFAGS) -c $^

clean:
	rm *.o
	rm *.dis
	rm test
	rm test.bin
