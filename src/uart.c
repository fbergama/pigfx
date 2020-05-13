#include "peri.h"
#include "utils.h"
#include "timer.h"
#include "console.h"
#include "uart.h"

void uart_init(void)
{
    // set TX to use no resistor, RX to use pull-up resistor
    W32(GPIO_PUD, 0);
    // wait 150us
    usleep(150);
    // set bit 14 in PUDCLK0 (set resistor state for pin 14 - TX)
    W32(GPIO_PUDCLK0, 1<<14);
    // wait 150us
    usleep(150);
    // write 0 to GPPUD
    W32(GPIO_PUD, 0);
    // clear PUDCLK0
    W32(GPIO_PUDCLK0, 0);
    // write 2 to GPPUD (use pull-up resistor)
    W32(GPIO_PUD, 2);
    // wait 150us
    usleep(150);
    // set bit 15 in PUDCLK0 (set resistor state for pin 15 - RX)
    W32(GPIO_PUDCLK0, 1<<15);
    // wait 150us
    usleep(150);
    // write 0 to GPPUD
    W32(GPIO_PUD, 0);
    // clear PUDCLK0
    W32(GPIO_PUDCLK0, 0);
    
    // Disable UART0:
    // clear UART0_CR = UART0_BASE + 0x30;
    W32(UART0_CR, 0);
    // Clear UART0 interrupts:
    // clear UART0_ICR = UART0_BASE + 0x44;
    W32(UART0_ICR, 0xFFFFFFFF);
    // Set 8bit (bit 6-5=1), no parity (bit 7=0), FIFO enable (bit 4=1)
    W32(UART0_LCRH, 0x70);
    // Enable TX(bit9) RX(bit8) and UART0(bit0)
    W32(UART0_CR, (1 << 0) | (1 << 8) | (1 << 9));
}

void uart_write(const char ch )
{
    // Wait for UART to become ready to transmit.
    while ( R32(UART0_FR) & (1 << 5) ) { }
    W32(UART0_DR, ch);
}

void uart_write_str(const char* data)
{
    for (unsigned int i=0; data[i] != 0; i++)
        uart_write((unsigned char)data[i]);
}

void byte2hex(char in, char* out)
{
    char hexChars[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                              'A', 'B', 'C', 'D', 'E', 'F' };
    out[2] = 0;
    out[1] = hexChars[in & 0x0F];
    out[0] = hexChars[(in & 0xF0) >> 4];
}

void uart_dump_mem(unsigned char* start_addr, unsigned char* end_addr)
{
    char tmp[3];
    char cntChars;
    if (start_addr>end_addr) return;  // that's bad
    
    do
    {
        // Print Address
        cout_h((unsigned int)start_addr);
        // space
        uart_write_str("  ");
        cntChars = 0;
        do
        {
            byte2hex(*start_addr, &tmp[0]);
            uart_write_str(tmp);
            cntChars++;
            start_addr++;
            if (cntChars == 16) uart_write_str("\r\n");
            else if (cntChars % 4 == 0)
            {
                uart_write(' ');
            }
        } while ((start_addr<=end_addr) && (cntChars<16));
        
    } while (start_addr<=end_addr);
    uart_write_str("\r\n");
}










