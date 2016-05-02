#include "uart.h"
#include "utils.h"
#include "timer.h"
#include "framebuffer.h"
#include "console.h"
#include "gfx.h"
#include "irq.h"
#include "dma.h"

#define GPFSEL1 0x20200004
#define GPSET0  0x2020001C
#define GPCLR0  0x20200028

unsigned int led_status;
volatile unsigned int* UART0_DR;
volatile unsigned int* UART0_ITCR;
volatile unsigned int* UART0_IMSC;
volatile unsigned int* UART0_FR;

#define UART_BUFFER_SIZE 100000
volatile unsigned char uart_buffer[ UART_BUFFER_SIZE ];
volatile unsigned char* uart_buffer_start;
volatile unsigned char* uart_buffer_end;
volatile unsigned char* uart_buffer_limit;


inline void uart_fill_queue()
{
    while( !( *UART0_FR & 0x10)/*uart_poll()*/)
    {
        *uart_buffer_end++ = (unsigned char)( *UART0_DR & 0xFF /*uart_read_byte()*/);

        if( uart_buffer_end >= uart_buffer_limit )
           uart_buffer_end = uart_buffer; 

        if( uart_buffer_end == uart_buffer_start )
        {
            uart_buffer_start++;
            if( uart_buffer_start >= uart_buffer_limit )
                uart_buffer_start = uart_buffer; 
        }
    }
}

void __attribute__((interrupt("IRQ"))) irq_handler_(void)
{
    uart_fill_queue();

    /* Clear UART0 interrupts */
    *UART0_ITCR = 0xFFFFFFFF;
}


void initialize_uart_irq()
{
    uart_buffer_start = uart_buffer_end = uart_buffer;
    uart_buffer_limit = &( uart_buffer[ UART_BUFFER_SIZE ] );

    UART0_DR   = (volatile unsigned int*)0x20201000;
    UART0_IMSC = (volatile unsigned int*)0x20201038;
    UART0_ITCR = (volatile unsigned int*)0x20201044;
    UART0_FR   = (volatile unsigned int*)0x20201018;

    *UART0_IMSC = (1<<4) | (1<<7) | (1<<9); // Masked interrupts: RXIM + FEIM + BEIM (See pag 188 of BCM2835 datasheet)
    *UART0_ITCR = 0xFFFFFFFF; // Clear UART0 interrupts

    pIRQController->Enable_IRQs_2 = RPI_UART_INTERRUPT_IRQ;
    enable_irq();
}


void heartbeat_init()
{
    unsigned int ra;
    ra=R32(GPFSEL1);
    ra&=~(7<<18);
    ra|=1<<18;
    W32(GPFSEL1,ra);

    led_status=0;
}


void heartbeat_loop()
{
    unsigned int last_time = 0;
    unsigned int curr_time;

    while(1)
    {
        /*
        if( uart_poll() )
        {
            unsigned int ch = uart_read_byte();
            if( ch=='h')
            {
                uart_write_str("\n");
            }  
        }
        */
        
        curr_time = time_microsec();
        if( curr_time-last_time > 500000 )
        {
            uart_write_str("AAA");
            if( led_status )
            {
                W32(GPCLR0,1<<16);
                led_status = 0;
            } else
            {
                W32(GPSET0,1<<16);
                led_status = 1;
            }
            last_time = curr_time;
        } 
    }

}

void initialize_framebuffer()
{
    //uart_write_str("Initializing video..");
    usleep(100000);
    fb_release();

    unsigned char* p_fb=0;
    unsigned int fbsize;
    unsigned int pitch;

    unsigned int p_w = 640;
    unsigned int p_h = 480;
    unsigned int v_w = p_w;
    unsigned int v_h = p_h;

    fb_init( p_w, p_h, 
             v_w, v_h,
             8, 
             (void*)&p_fb, 
             &fbsize, 
             &pitch );

    fb_set_xterm_palette();

    //cout("fb addr: ");cout_h((unsigned int)p_fb);cout_endl();
    //cout("fb size: ");cout_d((unsigned int)fbsize);cout(" bytes");cout_endl();
    //cout("  pitch: ");cout_d((unsigned int)pitch);cout_endl();

    if( fb_get_phisical_buffer_size( &p_w, &p_h ) != FB_SUCCESS )
    {
        cout("fb_get_phisical_buffer_size error");cout_endl();
    }
    //cout("phisical fb size: "); cout_d(p_w); cout("x"); cout_d(p_h); cout_endl();

    usleep(100000);
    gfx_set_env( p_fb, v_w, v_h, pitch, fbsize ); 
    gfx_clear();
}


void video_test()
{
    unsigned char ch='A';
    unsigned int row=0;
    unsigned int col=0;
    unsigned int term_cols, term_rows;
    gfx_get_term_size( &term_rows, &term_cols );

#if 0
    unsigned int t0 = time_microsec();
    for( row=0; row<1000000; ++row )
    {
        gfx_putc(0,col,ch);
    }
    t0 = time_microsec()-t0;
    cout("T: ");cout_d(t0);cout_endl();
    return;
#endif
#if 0
    while(1)
    {
        gfx_putc(row,col,ch);
        col = col+1;
        if( col >= term_cols )
        {
            usleep(100000);
            col=0;
            gfx_scroll_up(8);
        }
        ++ch;
        gfx_set_fg( ch );
    }
#endif
#if 1
    while(1)
    {
        gfx_putc(row,col,ch);
        col = col+1;
        if( col >= term_cols )
        {
            usleep(50000);
            col=0;
            row++;
            if( row >= term_rows )
            {
                row=term_rows-1;
                int i;
                for(i=0;i<10;++i)
                {
                    usleep(500000);
                    gfx_scroll_down(8);
                    gfx_set_bg( i );
                }
                usleep(1000000);
                gfx_clear();
                return;
            }

        }
        ++ch;
        gfx_set_fg( ch );
    }
#endif

#if 0
    while(1)
    {
        gfx_set_bg( RR );
        gfx_clear();
        RR = (RR+1)%16;
        usleep(1000000);
    }
#endif

}


void video_line_test()
{
    int x=-10; 
    int y=-10;
    int vx=1;
    int vy=0;

    gfx_set_fg( 15 );

    while(1)
    {
        // Render line
        gfx_line( 320, 240, x, y );

        usleep( 1000 );

        // Clear line
        gfx_swap_fg_bg();
        gfx_line( 320, 240, x, y );
        gfx_swap_fg_bg();

        x = x+vx;
        y = y+vy;
        
        if( x>700 )
        {
            x--;
            vx--;
            vy++;
        }
        if( y>500 )
        {
            y--;
            vx--;
            vy--;
        }
        if( x<-10 )
        {
            x++;
            vx++;
            vy--;
        }
        if( y<-10 )
        {
            y++;
            vx++;
            vy++;
        }

    }
}


void term_main_loop()
{
    gfx_term_putstring( (unsigned char*)"\x1B[2J" );
    gfx_term_putstring( (unsigned char*)"\x1B[30;35HPIGFX Ready!" );

    while( uart_buffer_start == uart_buffer_end )
        usleep(100000 );

    gfx_term_putstring( (unsigned char*)"\x1B[2J" );

    unsigned int t0 = time_microsec();
    unsigned char strb[2] = {0,0};

    while(1)
    {
        if( !DMA_CHAN0_BUSY && uart_buffer_start != uart_buffer_end )
        {
            strb[0] = *uart_buffer_start++;
            if( uart_buffer_start >= uart_buffer_limit )
                uart_buffer_start = uart_buffer;

            gfx_term_putstring( strb );
        }

        uart_fill_queue();

        if( time_microsec()-t0 > 500000 )
        {
            if( led_status )
            {
                W32(GPCLR0,1<<16);
                led_status = 0;
            } else
            {
                W32(GPSET0,1<<16);
                led_status = 1;
            }
            t0 = time_microsec();
        }

    }

}


void entry_point()
{
    uart_init();
    heartbeat_init();
    
    //heartbeat_loop();
    
    initialize_framebuffer();

    //video_test();
    //video_line_test();

    initialize_uart_irq();
    term_main_loop();
}
