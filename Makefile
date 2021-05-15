
RPI ?= 1

ARMGNU ?= arm-none-eabi
ifeq ($(strip $(RPI)),1)
CFLAGS = -Wall -Wextra -O2 -g -nostdlib -nostartfiles -fno-stack-limit -ffreestanding -fsigned-char -march=armv6j -mtune=arm1176jzf-s -DRPI=1
else ifeq ($(strip $(RPI)),2)
CFLAGS = -Wall -Wextra -O2 -g -nostdlib -nostartfiles -fno-stack-limit -ffreestanding -fsigned-char -march=armv7-a -mtune=cortex-a7 -DRPI=2
else ifeq ($(strip $(RPI)),3)
CFLAGS = -Wall -Wextra -O2 -g -nostdlib -nostartfiles -fno-stack-limit -ffreestanding -fsigned-char -march=armv8-a -mtune=cortex-a53 -DRPI=3
else
CFLAGS = -Wall -Wextra -O2 -g -nostdlib -nostartfiles -fno-stack-limit -ffreestanding -fsigned-char -march=armv8-a -mtune=cortex-a53 -DRPI=4
endif

## Important!!! asm.o must be the first object to be linked!
OOB = asm.o exceptionstub.o synchronize.o mmu.o pigfx.o uart.o irq.o utils.o gpio.o mbox.o prop.o board.o actled.o framebuffer.o console.o gfx.o dma.o nmalloc.o uspios_wrapper.o ee_printf.o stupid_timer.o block.o emmc.o c_utils.o mbr.o fat.o config.o ini.o ps2.o keyboard.o transfer.o binary_assets.o

BUILD_DIR = build
SRC_DIR = src
BUILD_VERSION = $(shell git describe --all --long | cut -d "-" -f 3)


OBJS=$(patsubst %.o,$(BUILD_DIR)/%.o,$(OOB))

LIBGCC=$(shell $(ARMGNU)-gcc -print-libgcc-file-name)

ifeq ($(strip $(RPI)),4)
LIBUSPI=
else
LIBUSPI=uspi/lib/libuspi.a
endif

all: pigfx.elf pigfx.hex kernel 
	ctags src/

$(SRC_DIR)/pigfx_config.h: pigfx_config.h.in 
	@sed 's/\$$VERSION\$$/$(BUILD_VERSION)/g' pigfx_config.h.in > $(SRC_DIR)/pigfx_config.h
	@echo "Creating pigfx_config.h"

run: pigfx.elf
	./launch_qemu.bash

kernel: pigfx.img
	@if [ $(RPI) == 1 ]; then \
		cp pigfx.img bin/kernel.img; \
	elif [ $(RPI) == 2 ]; then \
		cp pigfx.img bin/kernel7.img; \
	elif [ $(RPI) == 3 ]; then \
		cp pigfx.img bin/kernel8-32.img; \
	else \
		cp pigfx.img bin/recovery7l.img; \
	fi;


debug: pigfx.elf
	cd JTAG && ./run_gdb.sh

dump: pigfx.elf
	@$(ARMGNU)-objdump --disassemble-zeroes -D pigfx.elf > pigfx.dump
	@echo "OBJDUMP $<"

$(BUILD_DIR)/%.o : $(SRC_DIR)/%.c 
	@$(ARMGNU)-gcc $(CFLAGS) -c $< -o $@
	@echo "CC $<"

$(BUILD_DIR)/%.o : $(SRC_DIR)/%.S 
	@$(ARMGNU)-gcc $(CFLAGS) -c $< -o $@
	@echo "AS $<"

$(BUILD_DIR)/%.o : $(SRC_DIR)/%.s 
	@$(ARMGNU)-gcc $(CFLAGS) -c $< -o $@
	@echo "AS $<"

%.hex : %.elf 
	@$(ARMGNU)-objcopy $< -O ihex $@
	@echo "OBJCOPY $< -> $@"

%.img : %.elf 
	@$(ARMGNU)-objcopy $< -O binary $@
	@echo "OBJCOPY $< -> $@"

pigfx.elf : $(SRC_DIR)/pigfx_config.h $(OBJS) 
	@$(ARMGNU)-ld $(OBJS) $(LIBGCC) $(LIBUSPI) -T memmap -o $@
	@echo "LD $@"


.PHONY clean :
	rm -f $(SRC_DIR)/pigfx_config.h
	rm -f $(BUILD_DIR)/*.o
	rm -f *.hex
	rm -f *.elf
	rm -f *.img
	rm -f *.dump
	rm -f tags
