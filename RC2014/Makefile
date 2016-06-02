CLIBSDIR = "$(ZCCCFG)/../clibs/"
CINCDIR = "$(ZCCCFG)/../"



all: pigfx_test.bin
	cp pigfx_test.bin pigfx_test.rom


pigfx_test.bin: pigfx_test.asm pigfx.lib
	z80asm -l -a -m -L$(CLIBSDIR) -I$(CINCDIR) -ipigfx.lib -iz80_crt0.lib -o=pigfx_test.bin rc2014init.asm pigfx_test.asm

pigfx_test.asm: pigfx_test.c
	zcc +zx80 -nostdlib -SO0 -O0 -a -c -z80-verb -lpigfx $<

pigfx.lib: pigfx.asm
	z80asm -l -xpigfx.lib pigfx.asm

.PHONY clean:
	rm -f *.bin *.lst *.ihx *.obj *.rom  *.reloc *.sym *.map pigfx.lib *.opt pigfx_test.asm zcc_opt.def
