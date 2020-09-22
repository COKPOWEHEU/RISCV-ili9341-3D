progname := firmware
progdir := res
files := main startup_c startup lcd_ili9341 gl

srcdir := src
builddir := build
libdir := lib


CROSS_COMPILE=riscv64-unknown-elf-
CC=$(CROSS_COMPILE)gcc
AS=$(CROSS_COMPILE)gcc
LD=$(CROSS_COMPILE)gcc
OBJCOPY=$(CROSS_COMPILE)objcopy
OBJDUMP=$(CROSS_COMPILE)objdump
SIZE=$(CROSS_COMPILE)size

#LIBGCC=${shell ${CC} ${CFLAGS} -print-libgcc-file-name}
#LIBM=${shell ${CC} ${CFLAGS} -print-file-name=libm.a}

LSCRIPT=$(libdir)/gd32vf103.ld

CFLAGS:=-Wall -Os -g -Wno-main -Wstack-usage=400 -ffreestanding -Wno-unused -nostdlib
CFLAGS+=-fno-builtin-printf -march=rv32imac -mabi=ilp32 -mcmodel=medany
#CFLAGS+=-Iexternal/GD32VF103_Firmware_Library/Firmware/GD32VF103_standard_peripheral/Include
#CFLAGS+=-Iexternal/GD32VF103_Firmware_Library/Firmware/GD32VF103_standard_peripheral
#CFLAGS+=-Iexternal/GD32VF103_Firmware_Library/Template
#CFLAGS+=-Iexternal/GD32VF103_Firmware_Library/Firmware/RISCV/drivers
CFLAGS+=-I$(libdir)/Firmware/GD32VF103_standard_peripheral/Include
CFLAGS+=-I$(libdir)/Firmware/GD32VF103_standard_peripheral
CFLAGS+=-I$(libdir)/Firmware/RISCV/drivers
CFLAGS+=-I$(libdir)/Firmware
CFLAGS+=-I/usr/lib/picolibc/riscv64-unknown-elf/include/
CFLAGS+=-MD -MP -MT $(builddir)/$(*F).o -MF $(builddir)/dep/$(@F).mk
CFLAGS+=-DGD32VF103C_START
ASFLAGS:=$(CFLAGS)

LIBGCC=${shell ${CC} ${CFLAGS} -print-libgcc-file-name}
LIBM=${shell ${CC} ${CFLAGS} -print-file-name=libm.a}

LDFLAGS:=-T $(LSCRIPT) -Wl,-gc-sections -Wl,-Map=$(builddir)/$(progname).map -nostdlib -march=rv32imac -mabi=ilp32 \
         -mcmodel=medany -ffunction-sections -fdata-sections
         
frmname = $(progdir)/$(progname)
objs = $(addprefix $(builddir)/,$(addsuffix .o,$(files))) $(LIBGCC)

#test:
#	@echo $(CFLAGS)
#	@echo $(LIBGCC)
#	@echo $(LIBM)
all: $(frmname).bin $(frmname).hex $(frmname).lss size

$(frmname).bin: $(frmname).elf
	$(OBJCOPY) -O binary $^ $@
$(frmname).hex: $(frmname).elf
	$(OBJCOPY) -Oihex $(frmname).elf $(frmname).hex
$(frmname).lss: $(frmname).elf
	$(OBJDUMP) -D -S $(frmname).elf > $(frmname).lss

$(frmname).elf: $(objs) $(LSCRIPT)
	mkdir -p $(progdir)
	@ echo "..linking"
	$(LD) $(LDFLAGS) $(objs) -o $@ 

$(builddir)/%.o: $(srcdir)/%.c
	mkdir -p $(builddir)
	$(CC) -c $(CFLAGS) $< -o $@
$(builddir)/%.o: $(srcdir)/%.S
	mkdir -p $(builddir)
	$(CC) -c $(CFLAGS) $< -o $@
	
clean:
	rm -rf $(progdir)
	rm -rf $(builddir)
size: $(frmname).elf
	$(SIZE) $(frmname).elf
prog: $(frmname).bin
	stm32flash /dev/ttyUSB0 -w $(frmname).bin

-include $(shell mkdir -p $(builddir)/dep) $(wildcard $(builddir)/dep/*)

.PHONY: all clean
