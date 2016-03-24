#include "console.h"
#include "uart.h"
#include "utils.h"

void cout( char* str ) 
{
    uart_write_str(str);
}

void cout_endl()
{
    cout("\n");
}

void cout_h(unsigned int v)
{
    char buff[15]={0};
    buff[0]='0';
    buff[1]='x';
    word2hexstr( v, buff+2 ); 
    cout(buff);
}

void cout_d(unsigned int val)
{
    char buffer[32] = {0};
    unsigned int i = 0;
    unsigned int t;

    if(val == 0)
    {
        buffer[0] = '0';
        buffer[1] = '\0';
        cout(buffer);
        return;
    }   

    cout( buffer );

    while(val!=0)
    {
        buffer[i++] = val%10+'0';
        val=val/10;
    }

    buffer[i] = '\0';

    for(t = 0; t < i/2; t++)
    {
        buffer[t] ^= buffer[i-t-1];
        buffer[i-t-1] ^= buffer[t];
        buffer[t] ^= buffer[i-t-1];
    }

    cout(buffer);
}
