#include "peri.h"
#include "utils.h"
#include "timer.h"
#include "console.h"
#include "gpio.h"
#include "uart.h"

unsigned char actUart;

// Loop <delay> times in a way that the compiler won't optimize away
static inline void delay(unsigned int count)
{
	asm volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
		 : "=r"(count): [count]"0"(count) : "cc");
}

void uart_init(void)
{
    // set TX to use no resistor, RX to use pull-up resistor
    gpio_setpull(14, GPIO_PULL_OFF);    //set resistor state for pin 14 - TX -> no resistor
    gpio_setpull(15, GPIO_PULL_UP);     //set resistor state for pin 15 - RX -> pull up
    
    if (actUart == 1)
    {
        // USE UART1, because Bluetooth uses UART0
        W32(AUX_ENABLES,1);     // Enable Mini Uart
        W32(AUX_MU_IER_REG,0);  // Clear FIFO interrupts
        W32(AUX_MU_CNTL_REG,0); // Disable Extra control functions
        W32(AUX_MU_LCR_REG,3);  // Set Uart to 8-bit mode
        W32(AUX_MU_MCR_REG,0);
        W32(AUX_MU_IIR_REG,0xC6);   // disable interrupts, reset FIFO
        W32(AUX_MU_BAUD_REG,270);   // baudrate 115200 TODO cannot be set from config.txt for now
        W32(AUX_MU_IER_REG,13);  // enable rx interrupts
        
        gpio_select(14, GPIO_FUNCTION_5);
        gpio_select(15, GPIO_FUNCTION_5);
        
        W32(AUX_MU_CNTL_REG,3); // enable Tx, Rx
    }
    else {
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
    }
}

void uart_write(const char ch )
{
    // Wait for UART to become ready to transmit.
    if (actUart == 1)
    {
        while(1)
        {
            if(R32(AUX_MU_LSR_REG)&(1 << 5)) break;
        }
        W32(AUX_MU_IO_REG, ch);
    }
    else
    {
        while ( R32(UART0_FR) & (1 << 5) ) { }
        W32(UART0_DR, ch);
    }
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
