#include "pigfx.h"

void fun()
{
    unsigned int i;
    pigfx_fgcol(3);

    for( i=0; i<10; ++i )
    {
        pigfx_fgcol(i);
        pigfx_print(" RC2014 ");
    }


    while(1)
        continue;
}
