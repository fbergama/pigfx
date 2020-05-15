#include "peri.h"
#include "utils.h"
#include "timer.h"
#include "console.h"
#include "uart.h"

// Loop <delay> times in a way that the compiler won't optimize away
static inline void delay(unsigned int count)
{
	asm volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
		 : "=r"(count): [count]"0"(count) : "cc");
}

void uart_init(void)
{
    // set TX to use no resistor, RX to use pull-up resistor
    W32(GPIO_PUD, 0);
    // wait 150us
    usleep(150);
    // set bit 14 in PUDCLK0 (set resistor state for pin 14 - TX)
    W32(GPIO_PUDCLK0, 1<<14);
    // wait 150 cycles
    delay(150);
    // write 0 to GPPUD
    W32(GPIO_PUD, 0);
    // clear PUDCLK0
    W32(GPIO_PUDCLK0, 0);
    // write 2 to GPPUD (use pull-up resistor)
    W32(GPIO_PUD, 2);
    // wait 150 cycles
    delay(150);
    // set bit 15 in PUDCLK0 (set resistor state for pin 15 - RX)
    W32(GPIO_PUDCLK0, 1<<15);
    // wait 150 cycles
    delay(150);
    // write 0 to GPPUD
    W32(GPIO_PUD, 0);
    // clear PUDCLK0
    W32(GPIO_PUDCLK0, 0);
    
#if RPI>=3
    // USE UART1, because Bluetooth uses UART0
    unsigned int ra;

    W32(AUX_ENABLES,1);     // Enable Mini Uart
    W32(AUX_MU_IER_REG,0);  // Clear FIFO interrupts
    W32(AUX_MU_CNTL_REG,0); // Disable Extra control functions
    W32(AUX_MU_LCR_REG,3);  // Set Uart to 8-bit mode
    W32(AUX_MU_MCR_REG,0);
    W32(AUX_MU_IER_REG,0);  // interrupt disabled
    W32(AUX_MU_IIR_REG,0xC6);   // disable interrupts, reset FIFO
    W32(AUX_MU_BAUD_REG,270);   // baudrate 115200
    ra=R32(GPIO_FSEL1);
    ra&=~(7<<12); //gpio14
    ra|=2<<12;    //alt5
    ra&=~(7<<15); //gpio15
    ra|=2<<15;    //alt5
    W32(GPIO_FSEL1,ra);
    W32(AUX_MU_CNTL_REG,3); // enable Tx, Rx
#else
    // Standard UART0 RPI1/2
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
#endif
}

void uart_write(const char ch )
{
    // Wait for UART to become ready to transmit.
#if RPI>=3
    while(1)
    {
        if(R32(AUX_MU_LSR_REG)&(1 << 5)) break;
    }
    W32(AUX_MU_IO_REG, ch);
#else
    while ( R32(UART0_FR) & (1 << 5) ) { }
    W32(UART0_DR, ch);
#endif
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

unsigned int uart0_get_baudrate()
{
    // This only works with a non Bluetooth model, so not for Pi Zero W, 3, 4
    // Default UART Clock is 48 MHz unless otherwise configured
    unsigned int ibrd = R32(UART0_IBRD);        // Divisor
    unsigned int fbrd = R32(UART0_FBRD);        // Fractional part
    unsigned int divider, baudrate;
    unsigned int baud, multi;
    
    // Uart clock gets divided by 16. This is the base for further division
    // 48 MHz divided by 16 is 3 MHz
    // For example with a baudrate of 115200 the divider is 26, the fractional is 3. This is 26 + 3/64 = 26.046875
    // divider = ibrd + fbrd / 64;
    // as we don't want to use float, we multiply by 64
    divider = ibrd*64 + fbrd;
    baudrate = 3000000*64 / divider;
    // this is the real baudrate (115176)
    // now we add 150 for beeing able to calculate the configured baudrate by rounding down to the lower 300 step
    baudrate += 150;
    multi = (unsigned int)baudrate / 300;
    baud = 300 * multi;     //115200*/
    
    return baud;
}








