cmd_arch/arm/lib/copy_page.o := arm-linux-gcc -Wp,-MD,arch/arm/lib/.copy_page.o.d  -nostdinc -isystem /usr/local/arm_linux_4.8/lib/gcc/arm-nuvoton-linux-uclibceabi/4.8.4/include -I/home/wuyu/nuc970bsp/kernel/arch/arm/include -Iarch/arm/include/generated  -Iinclude -I/home/wuyu/nuc970bsp/kernel/arch/arm/include/uapi -Iarch/arm/include/generated/uapi -I/home/wuyu/nuc970bsp/kernel/include/uapi -Iinclude/generated/uapi -include /home/wuyu/nuc970bsp/kernel/include/linux/kconfig.h -D__KERNEL__ -mlittle-endian -Iarch/arm/mach-nuc970/include  -D__ASSEMBLY__ -mabi=aapcs-linux -mno-thumb-interwork -marm -D__LINUX_ARM_ARCH__=5 -march=armv5te -mtune=arm9tdmi -include asm/unified.h -msoft-float         -c -o arch/arm/lib/copy_page.o arch/arm/lib/copy_page.S

source_arch/arm/lib/copy_page.o := arch/arm/lib/copy_page.S

deps_arch/arm/lib/copy_page.o := \
  /home/wuyu/nuc970bsp/kernel/arch/arm/include/asm/unified.h \
    $(wildcard include/config/arm/asm/unified.h) \
    $(wildcard include/config/thumb2/kernel.h) \
  include/linux/linkage.h \
  include/linux/compiler.h \
    $(wildcard include/config/sparse/rcu/pointer.h) \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/enable/warn/deprecated.h) \
    $(wildcard include/config/kprobes.h) \
  include/linux/stringify.h \
  include/linux/export.h \
    $(wildcard include/config/have/underscore/symbol/prefix.h) \
    $(wildcard include/config/modules.h) \
    $(wildcard include/config/modversions.h) \
    $(wildcard include/config/unused/symbols.h) \
  /home/wuyu/nuc970bsp/kernel/arch/arm/include/asm/linkage.h \
  /home/wuyu/nuc970bsp/kernel/arch/arm/include/asm/assembler.h \
    $(wildcard include/config/cpu/feroceon.h) \
    $(wildcard include/config/trace/irqflags.h) \
    $(wildcard include/config/smp.h) \
    $(wildcard include/config/cpu/use/domains.h) \
  /home/wuyu/nuc970bsp/kernel/arch/arm/include/asm/ptrace.h \
    $(wildcard include/config/arm/thumb.h) \
  /home/wuyu/nuc970bsp/kernel/arch/arm/include/uapi/asm/ptrace.h \
    $(wildcard include/config/cpu/endian/be8.h) \
  /home/wuyu/nuc970bsp/kernel/arch/arm/include/asm/hwcap.h \
  /home/wuyu/nuc970bsp/kernel/arch/arm/include/uapi/asm/hwcap.h \
  /home/wuyu/nuc970bsp/kernel/arch/arm/include/asm/domain.h \
    $(wildcard include/config/io/36.h) \
  /home/wuyu/nuc970bsp/kernel/arch/arm/include/asm/opcodes-virt.h \
  /home/wuyu/nuc970bsp/kernel/arch/arm/include/asm/opcodes.h \
    $(wildcard include/config/cpu/endian/be32.h) \
  /home/wuyu/nuc970bsp/kernel/arch/arm/include/asm/asm-offsets.h \
  include/generated/asm-offsets.h \
  /home/wuyu/nuc970bsp/kernel/arch/arm/include/asm/cache.h \
    $(wildcard include/config/arm/l1/cache/shift.h) \
    $(wildcard include/config/aeabi.h) \

arch/arm/lib/copy_page.o: $(deps_arch/arm/lib/copy_page.o)

$(deps_arch/arm/lib/copy_page.o):
