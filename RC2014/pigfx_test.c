#include "pigfx.h"
#include "rc2014.h"


void entry_point_()
{
    unsigned int i;
    //pigfx_fgcol(3);

    rc2014_putc('T');
    pigfx_printnum(123);

    for( i=0; i<50; ++i )
    {
        //pigfx_fgcol(i);
        pigfx_printnum(i);pigfx_print("\r\n");
    }

    while(1);
}

void aa()
{
    while(1);
}
