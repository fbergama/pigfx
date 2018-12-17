#include "console.h"
#include "uart.h"
#include "utils.h"

/** Outputs a zero-ending character string to UART.
 * @param str the string
 */
void cout( const char* str ) 
{
    uart_write_str(str);
}

/** Outputs an CR character to UART.
 *
 */
void cout_endl()
{
	cout("\x0D"); //cout("\n");
}

/** Outputs a 32-bits integer to UART as hexadecimal 0xXXXXXXXXX string.
 * @param val a 32-bits integer
 */
void cout_h( unsigned int val )
{
	// buffer needs 11 bytes : "0x" + hexa translation "XXXXXXXXX" + ending "\0"
    char buff[16]; // = {0} would generate a reference to memset()
    buff[0]='0';
    buff[1]='x';
    for (int i = 2 ; i < 16 ; i++) buff[i] = 0;
    word2hexstr( val, buff+2 );
    cout(buff);
}

/** Outputs a 32-bits integer to UART as a decimal number string.
 * @param val a 32-bits integer.
 */
void cout_d( unsigned int val )
{
    char buffer[32]; // = {0}; // generates a call to memset, avoid
    unsigned int i = 0;
    unsigned int t;
    for (i = 0 ; i < 32 ; i++) buffer[i]=0;

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
