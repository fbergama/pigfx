
ARMGNU ?= arm-none-eabi
CFLAGS = -Wall -Wextra -O0 -g -nostdlib -nostartfiles -fno-stack-limit -ffreestanding -mfloat-abi=hard


## Important!!! asm.o must be the first object to be linked!
OOB = asm.o pigfx.o uart.o irq.o utils.o timer.o framebuffer.o postman.o console.o gfx.o dma.o nmalloc.o uspios_wrapper.o ee_printf.o raspihwconfig.o stupid_timer.o binary_assets.o

BUILD_DIR = build
SRC_DIR = src


OBJS=$(patsubst %.o,$(BUILD_DIR)/%.o,$(OOB))

LIBGCC=$(shell $(ARMGNU)-gcc -print-libgcc-file-name)
LIBUSPI=uspi/lib/libuspi.a

all: pigfx.elf pigfx.hex kernel 
	ctags src/

run: pigfx.elf
	./launch_qemu.bash

kernel: pigfx.img
	cp pigfx.img bin/kernel.img

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

pigfx.elf : $(OBJS) 
	$(ARMGNU)-ld $(OBJS) $(LIBGCC) $(LIBUSPI) -T memmap -o $@
	@echo "LD $@"


.PHONY clean :
	rm -f $(BUILD_DIR)/*.o
	rm -f *.hex
	rm -f *.elf
	rm -f *.img
	rm -f *.dump
	rm -f tags
