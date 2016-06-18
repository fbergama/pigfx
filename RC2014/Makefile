
all: pigfx_test.hex TinyBasicPlus.hex
#	z80dasm -g 0 -a pigfx_test.rom > disasm.txt



TinyBasicPlus_CODE.bin: TinyBasicPlus.c pigfx.lib
	zcc +embedded -vn -m -clib=new -startup=1 -lpigfx.lib -o TinyBasicPlus rc2014init.asm TinyBasicPlus.c


pigfx_test_CODE.bin: pigfx_test.c pigfx.lib
	zcc +embedded -vn -m -clib=new -startup=1 -lpigfx.lib -o pigfx_test rc2014init.asm pigfx_test.c


pigfx_test.rom: pigfx_test_CODE.bin 
	cat pigfx_test_INIT.bin > pigfx_test.rom
	cat pigfx_test_CODE.bin >> pigfx_test.rom
	cat pigfx_test_DATA.bin >> pigfx_test.rom

TinyBasicPlus.rom: TinyBasicPlus_CODE.bin 
	cat TinyBasicPlus_INIT.bin > TinyBasicPlus.rom
	cat TinyBasicPlus_CODE.bin >> TinyBasicPlus.rom
	cat TinyBasicPlus_DATA.bin >> TinyBasicPlus.rom

%.hex : %.rom 
	appmake +hex -b $< -o $@

pigfx.lib: pigfx.asm
	z80asm -l -xpigfx.lib pigfx.asm

.PHONY clean:
	rm -f *.bin *.lst *.ihx *.hex *.obj *.rom  *.reloc *.sym *.map pigfx.lib *.opt pigfx_test.asm  pigfx_test disasm.txt TinyBasicPlus TinyBasicPlus.asm zcc_opt.def



