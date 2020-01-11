TARGET      := iis
TARGET_BIN := $(TARGET).bin
TARGET_ELF := $(TARGET).elf
TARGET_DIS  := $(TARGET).dis
TARGET_LDS  := $(TARGET).lds

CROSS_COMPILE := arm-linux-
CC      = $(CROSS_COMPILE)gcc
LD      = $(CROSS_COMPILE)ld
AR      = $(CROSS_COMPILE)ar
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump

CROSS_PATH := /usr/local/arm_linux_4.8/lib/gcc/arm-nuvoton-linux-uclibceabi/4.8.4


INCLUDEDIR1 	:= $(shell pwd)/include 
INCLUDEDIR2     := $(shell pwd)/driver/include
CFLAGS 		    := -Wall -O2
CPPFLAGS   	    := -nostdinc -I$(INCLUDEDIR1) -I$(INCLUDEDIR2) -I $(CROSS_PATH)/include
LDFLAGS         := -lgcc -L$(CROSS_PATH) -lgcc

export CC LD OBJCOPY OBJDUMP CFLAGS CPPFLAGS AR
export INCLUDEDIR2 INCLUDEDIR1 	

drv         := driver/lib
driver_src := $(drv)/system_nuc970.c $(drv)/sys_uart.c $(drv)/gpio.c $(drv)/sys.c
driver_obj := $(patsubst %.c, %.o, $(driver_src))

objs := head.o $(driver_obj) main.o lib/libc.a

$(TARGET_BIN): $(objs)
	${LD} -T$(TARGET_LDS) -o $(TARGET_ELF) $^ $(LDFLAGS)
	${OBJCOPY} -O binary -S $(TARGET_ELF) $@
	${OBJDUMP} -D -m arm $(TARGET_ELF) > $(TARGET_DIS)

.PHONY : lib/libc.a
lib/libc.a:
	cd lib; make; cd ..
	
%.o:%.c
	${CC} $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

%.o:%.S
	${CC} $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

clean:
	make  clean -C lib
	rm -f $(TARGET_BIN)
	rm -f $(TARGET_ELF)
	rm -f $(TARGET_DIS)
	rm -f $(shell find -name "*.o")

	


