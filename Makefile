
ARMGNU ?= arm-none-eabi
CFLAGS = -Wall -Wextra -O0 -g -nostdlib -nostartfiles -fno-stack-limit -ffreestanding -mfloat-abi=hard

## Important!!! asm.o must be the first object to be linked!
OOB = asm.o pigfx.o uart.o irq.o utils.o timer.o framebuffer.o postman.o console.o gfx.o dma.o nmalloc.o uspios_wrapper.o ee_printf.o raspihwconfig.o stupid_timer.o binary_assets.o
OOB_38400 = asm.o pigfx.o uart_38400.o irq.o utils.o timer.o framebuffer.o postman.o console.o gfx.o dma.o nmalloc.o uspios_wrapper.o ee_printf.o raspihwconfig.o stupid_timer.o binary_assets.o

BUILD_DIR = build
SRC_DIR = src
BUILD_VERSION = $(shell git describe --all --long | cut -d "-" -f 3)


OBJS=$(patsubst %.o,$(BUILD_DIR)/%.o,$(OOB))
OBJS_38400=$(patsubst %.o,$(BUILD_DIR)/%.o,$(OOB_38400))

LIBGCC=$(shell $(ARMGNU)-gcc -print-libgcc-file-name)
LIBUSPI=uspi/lib/libuspi.a

all: pigfx.elf pigfx_38400.elf pigfx.hex kernel 
	ctags src/

$(SRC_DIR)/pigfx_config.h: pigfx_config.h.in 
	@sed 's/\$$VERSION\$$/$(BUILD_VERSION)/g' pigfx_config.h.in > $(SRC_DIR)/pigfx_config.h
	@echo "Creating pigfx_config.h"

run: pigfx.elf
	./launch_qemu.bash

kernel: pigfx.img pigfx_38400.img
	cp pigfx.img bin/kernel.img
	cp pigfx_38400.img bin/38400/kernel.img

debug: pigfx.elf
	cd JTAG && ./run_gdb.sh

dump: pigfx.elf
	@$(ARMGNU)-objdump --disassemble-zeroes -D pigfx.elf > pigfx.dump
	@echo "OBJDUMP $<"

$(BUILD_DIR)/%.o : $(SRC_DIR)/%.c 
	@$(ARMGNU)-gcc $(CFLAGS) -c $< -o $@
	@echo "CC $<"

$(BUILD_DIR)/%.o : $(SRC_DIR)/%.s 
	@$(ARMGNU)-as $< -o $@
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
	
pigfx_38400.elf : $(SRC_DIR)/pigfx_config.h $(OBJS_38400) 
	@$(ARMGNU)-ld $(OBJS_38400) $(LIBGCC) $(LIBUSPI) -T memmap -o $@
	@echo "LD $@"


.PHONY clean :
	rm -f $(SRC_DIR)/pigfx_config.h
	rm -f $(BUILD_DIR)/*.o
	rm -f *.hex
	rm -f *.elf
	rm -f *.img
	rm -f *.dump
	rm -f tags
