all:

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
CFLAGS 		    := -Wall -O0
CPPFLAGS   	    := -nostdinc -I$(INCLUDEDIR1) -I$(INCLUDEDIR2) -I$(CROSS_PATH)/include
LDFLAGS         := -lgcc -L$(CROSS_PATH) -lgcc

export CC LD OBJCOPY OBJDUMP CFLAGS CPPFLAGS AR
export INCLUDEDIR2 INCLUDEDIR1 	

drv         := driver/lib
driver_src := $(drv)/system_nuc970.c $(drv)/sys_uart.c $(drv)/gpio.c $(drv)/sys.c
driver_obj := $(patsubst %.c, %.o, $(driver_src))


c_objs := $(driver_obj) init.o sys_nuc970_gcc.o main.o
s_objs := head.o
objs    := $(s_objs) $(c_objs) lib/libc.a
_dep_files := $(foreach f, $(c_objs), $(f).d)
dep_files := $(wildcard $(_dep_files))

ifneq ($(dep_files), )
	include $(dep_files)
endif



.PHONY : lib/libc.a
lib/libc.a:
	cd lib; make; cd ..
	
all: $(TARGET_BIN)

$(TARGET_BIN): $(objs)
	${LD} -T$(TARGET_LDS) -o $(TARGET_ELF) $^ $(LDFLAGS)
	${OBJCOPY} -O binary -S $(TARGET_ELF) $@
	${OBJDUMP} -D -m arm $(TARGET_ELF) > $(TARGET_DIS)

dep_file = -Wp,-MD,$@.d

%.o:%.c 
	${CC} $(CPPFLAGS) $(CFLAGS) -Wp,-MD,$(dep_file) -c -o $@ $<

%.o:%.S
	${CC} $(CPPFLAGS) $(CFLAGS) -Wp,-MD,$(dep_file) -c -o $@ $<

clean:
	echo $(dep_files)
	make  clean -C lib
	rm -f $(TARGET_BIN)
	rm -f $(TARGET_ELF)
	rm -f $(TARGET_DIS)
	rm -f $(shell find -name "*.o")
distclean:
	make  clean -C lib
	rm -f $(TARGET_BIN)
	rm -f $(TARGET_ELF)
	rm -f $(TARGET_DIS)
	rm -f $(shell find -name "*.o")
	rm -f $(shell find -name "*.d")




