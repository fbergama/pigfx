import os;
import math;
import sys;

BIN_NAME = "hexload.bin"
mem = 0xF800

bin_size = os.stat(BIN_NAME).st_size;
grp8 = int( math.ceil( bin_size / 4.0 ) );

print "clear"
print "10 let mb="+str(mem-65536);
print "20 for l=1 to "+str( grp8 ) 
print "30 input \"\";a,b,c,d"
print "31 poke mb, a"
print "32 poke mb+1, b"
print "33 poke mb+2, c"
print "34 poke mb+3, d"
print "35 let mb=mb+4"
print "40 next l"
print "50 poke "+str(0x8048-65536) + "," + str(0xC3)
print "60 poke "+str(0x8048-65536+1) + "," + str(0x00)
print "70 poke "+str(0x8048-65536+2) + "," + str(0xF8)
print "run"

with open(BIN_NAME, "rb") as f:

    bts = f.read(4)

    while len(bts) >= 1:

        while len(bts) < 4:
            bts = bts + chr(0)

        for kk in range(0, len(bts)-1):
            sys.stdout.write( str( ord(bts[kk]) )+",")

        sys.stdout.write( str( ord(bts[len(bts)-1])) )
        print ""
        bts = f.read(4)


print "print usr(0)";
