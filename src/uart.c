#include "peri.h"
#include "utils.h"
#include "timer.h"
#include "console.h"
#include "gpio.h"
#include "uart.h"
#include "ee_printf.h"
#include "mbox.h"

void uart_init(unsigned char configSpeed)
{
    // set TX to use no resistor, RX to use pull-up resistor
    gpio_setpull(14, GPIO_PULL_OFF);    //set resistor state for pin 14 - TX -> no resistor
    gpio_setpull(15, GPIO_PULL_UP);     //set resistor state for pin 15 - RX -> pull up
    
    W32(AUX_ENABLES,0);     // Disable Mini Uart
        
    gpio_select(14, GPIO_FUNCTION_0);       // Uart0
    gpio_select(15, GPIO_FUNCTION_0);       // Uart0
    
    // Disable UART0:
    // clear UART0_CR = UART0_BASE + 0x30;
    W32(UART0_CR, 0);
    
    if (configSpeed == 1)
    {
        typedef struct {
            mbox_msgheader_t header;
            mbox_tagheader_t tag;
        
            union {
                struct {
                    unsigned int clock_id;
                    unsigned int rate;  // hz
                    unsigned int skip_turbo;
                }
                request;
                struct {
                    unsigned int clock_id;
                    unsigned int rate;  // hz
                }
                response;
            }
            value;

            mbox_msgfooter_t footer;
        }
        message_t;

        message_t msg __attribute__((aligned(16)));

        msg.header.size = sizeof(msg);
        msg.header.code = 0;
        msg.tag.id = MAILBOX_TAG_SET_CLOCK_RATE; // Get board serial.
        msg.tag.size = sizeof(msg.value);
        msg.tag.code = 0;
        msg.value.request.clock_id = 2;     // UART Clock
        msg.value.request.rate = 48000000;     // 48 MHz
        msg.value.request.skip_turbo = 0;     // don't need this
        msg.footer.end = 0;

        if (mbox_send(&msg) != 0) {
            // oops
        }
        
        W32(UART0_IBRD, 26);    // 115200
        W32(UART0_FBRD, 3);     // 115200
    }

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

unsigned int uart_get_baudrate()
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
