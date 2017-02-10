#ifndef _BUFFERED_UART_H_
#define _BUFFERED_UART_H_


extern void buart_fill_queue( __attribute__((unused)) void* data );
extern void buart_initialize_irq();
extern int  buart_buffer_empty();
extern char buart_next_char();
extern void buart_purge();

#endif
