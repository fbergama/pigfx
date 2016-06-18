#include "rc2014.h"
#include "pigfx.h"


#pragma output CRT_ORG_CODE = 108
#pragma output REGISTER_SP  = -1
#pragma output CLIB_MALLOC_HEAP_SIZE = 0 


void entry_point_()
{
    int i;

    for( i=0; i<20; ++i )
    {
        pigfx_fgcol(i);
        pigfx_bgcol(100-i);
        pigfx_print("HELLO");
    }

}

void main()
{
#asm
    ld a, 'A'
    rst $8
#endasm
     
    entry_point_();
    while(1);

}
