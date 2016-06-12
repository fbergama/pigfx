#CLIBSDIR = "$(ZCCCFG)/../clibs/"
#CINCDIR = "$(ZCCCFG)/../"



#all: pigfx_test.bin TinyBasicPlus.bin
#	cp pigfx_test.bin pigfx_test.rom
#	cp TinyBasicPlus.bin TinyBasicPlus.rom
#
#
##pigfx_test.bin: pigfx_test.asm pigfx.lib
##	z80asm -l -a -m -L$(CLIBSDIR) -I$(CINCDIR) -ipigfx.lib -iz80_crt0.lib -o=pigfx_test.bin rc2014init.asm pigfx_test.asm
#
#
#TinyBasicPlus.bin: TinyBasicPlus.asm pigfx.lib
#	z80asm -l -a -m -L$(CLIBSDIR) -I$(CINCDIR) -ipigfx.lib -iz80_crt0.lib -o=TinyBasicPlus.bin rc2014init.asm TinyBasicPlus.asm
#
#TinyBasicPlus.asm: TinyBasicPlus.c
#	zcc +zx80 -nostdlib -SO0 -O0 -a -c -z80-verb $<
#
##pigfx_test.asm: pigfx_test.c
##	zcc +embedded -clib=new -startup=1 -nostdlib -SO0 -O0 -a -c -z80-verb $<

all: pigfx_test.rom TinyBasicPlus.rom
	z80dasm -g 0 -a pigfx_test.rom > disasm.txt


#rc2014init.bin: rc2014init.asm
#	z80asm -l -a rc2014init.asm	


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

pigfx.lib: pigfx.asm
	z80asm -l -xpigfx.lib pigfx.asm

.PHONY clean:
	rm -f *.bin *.lst *.ihx *.obj *.rom  *.reloc *.sym *.map pigfx.lib *.opt pigfx_test.asm  pigfx_test disasm.txt TinyBasicPlus TinyBasicPlus.asm zcc_opt.def
