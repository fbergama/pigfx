
ARMGNU ?= arm-none-eabi
COPS = -Wall -O0 -g -nostdlib -nostartfiles -ffreestanding 

## Important!!! asm.o must be the first object to be linked!
OBJS = asm.o pigfx.o uart.o utils.o timer.o framebuffer.o postman.o console.o gfx.o binary_assets.o

all: pigfx.elf pigfx.hex pigfx.img 
	ctags .

run: pigfx.elf
	./launch_qemu.bash

kernel: pigfx.img
	cp pigfx.img kernel.img

%.o : %.c 
	$(ARMGNU)-gcc $(COPS) -c $< -o $@

%.o : %.s 
	$(ARMGNU)-as $< -o $@

%.hex : %.elf 
	$(ARMGNU)-objcopy $< -O ihex $@

%.img : %.elf 
	$(ARMGNU)-objcopy $< -O binary $@


pigfx.elf : $(OBJS)
	$(ARMGNU)-ld $(OBJS) -T memmap -o $@
	$(ARMGNU)-objdump --disassemble-zeroes -D pigfx.elf > pigfx.dump


.PHONY clean :
	rm -f *.o
	rm -f *.hex
	rm -f *.elf
	rm -f *.img
	rm -f *.dump
	rm -f tags
