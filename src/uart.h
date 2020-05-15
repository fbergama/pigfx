#ifndef _PIGFX_UART_H_
#define _PIGFX_UART_H_


extern void uart_init(void);
extern void uart_write(const char ch );
extern void uart_write_str(const char* data);
extern void uart_dump_mem(unsigned char* start_addr, unsigned char* end_addr);
extern unsigned int uart0_get_baudrate();

extern unsigned char actUart;   // The Uart we are using

#endif
