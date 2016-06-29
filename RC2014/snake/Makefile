
all: snake.hex
#	z80dasm -g 0 -a pigfx_test.rom > disasm.txt



snake_CODE.bin: snake.c ../pigfx.lib
	zcc +embedded -vn -m -clib=new -startup=1 -l../pigfx.lib -o snake ../rc2014init.asm snake.c


snake.rom: snake_CODE.bin 
	cat snake_INIT.bin > snake.rom
	cat snake_CODE.bin >> snake.rom
	cat snake_DATA.bin >> snake.rom

%.hex : %.rom 
	appmake +hex -b $< -o $@


.PHONY clean:
	rm -f *.bin *.lst *.ihx *.hex *.obj *.rom zcc_opt.def snake *.reloc *.sym *.map 



