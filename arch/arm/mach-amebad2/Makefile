# Realtek Semiconductor Corp.
#
# bsp/Makefile
#     bsp Makefile
#
# Copyright (C) 2006-2012 Tony Wu (tonywu@realtek.com)
#

#
# Ameba
#
core-y		+= arch/arm/mach-amebad2/

KBUILD_CPPFLAGS += -I$(srctree)/arch/arm/mach-amebad2/ -I$(srctree)/arch/arm/mach-amebad2/include

obj-y := setup.o
obj-$(CONFIG_SMP) += smp.o headsmp.o
obj-$(CONFIG_HOTPLUG_CPU) += hotplug.o
obj-$(CONFIG_SUSPEND) += pm.o

##
## uncomment following lines for customized linker script
##
#CPPFLAGS_vmlinux.lds := -DTEXT_OFFSET=$(TEXT_OFFSET)
#export KBUILD_LDS := arch/arm64/bsp/vmlinux.lds
#extra-y += vmlinux.lds
