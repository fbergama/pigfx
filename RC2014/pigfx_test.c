#include "rc2014.h"
#include "pigfx.h"


#pragma output CRT_ORG_CODE = 108
#pragma output REGISTER_SP  = -1
#pragma output CLIB_MALLOC_HEAP_SIZE = 0 


void entry_point_()
{
    int i=0;
    
    pigfx_cls();
    pigfx_hide_cursor();
    for( i=0; i<5; ++i )
    {
        pigfx_movecursor( i, i );
        pigfx_print("HELLO");
    }
    pigfx_show_cursor();

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
