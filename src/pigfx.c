#include "uart.h"
#include "utils.h"
#include "timer.h"
#include "framebuffer.h"
#include "console.h"
#include "gfx.h"

#define GPFSEL1 0x20200004
#define GPSET0  0x2020001C
#define GPCLR0  0x20200028

unsigned int led_status;

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
    usleep(500000);
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

    usleep(500000);
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
                gfx_scroll_down(8);
                //gfx_clear();
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


#define UART_BUFF_SIZE 3
void term_main_loop()
{
    unsigned char buff[UART_BUFF_SIZE];

    gfx_term_putstring( (unsigned char*)"\x1B[2J" );
    gfx_term_putstring( (unsigned char*)"\x1B[30;35HPIGFX Ready!" );

    while( !uart_poll() )
        usleep(100000 );

    gfx_term_putstring( (unsigned char*)"\x1B[2J" );

    unsigned int t0 = time_microsec();
    unsigned int istart = 0;
    unsigned int iend = 0;
    unsigned char strb[2] = {0,0};

    while(1)
    {
        while( uart_poll() )
        {
            buff[ iend++ ] = uart_read_byte();
            
            if( iend >= UART_BUFF_SIZE )
                iend = 0;

            if( iend == istart )
            {
                ++istart;
                if( istart >= UART_BUFF_SIZE )
                    istart = 0;
            }
        }

        if( istart != iend )
        {
            strb[0] = buff[istart++];
            if( istart >= UART_BUFF_SIZE )
                istart = 0;

            gfx_term_putstring( strb );
        }


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
    term_main_loop();
}
