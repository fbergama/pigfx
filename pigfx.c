#include "uart.h"
#include "utils.h"
#include "timer.h"
#include "framebuffer.h"
#include "console.h"
#include "gfx.h"

#define GPFSEL1 0x20200004
#define GPSET0  0x2020001C
#define GPCLR0  0x20200028


void heartbeat_init()
{
    unsigned int ra;
    ra=R32(GPFSEL1);
    ra&=~(7<<18);
    ra|=1<<18;
    W32(GPFSEL1,ra);
}

void heartbeat_loop()
{
    unsigned int last_time = 0;
    unsigned int led_status = 0;
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
            uart_write_str("B");
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
    uart_write_str("Initializing video..");
    usleep(1000000);
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

    //fb_set_grayscale_palette();
    fb_set_xterm_palette();

    cout("fb addr: ");cout_h((unsigned int)p_fb);cout_endl();
    cout("fb size: ");cout_d((unsigned int)fbsize);cout(" bytes");cout_endl();
    cout("  pitch: ");cout_d((unsigned int)pitch);cout_endl();

    if( fb_get_phisical_buffer_size( &p_w, &p_h ) != FB_SUCCESS )
    {
        cout("fb_get_phisical_buffer_size error");cout_endl();
    }
    cout("phisical fb size: "); cout_d(p_w); cout("x"); cout_d(p_h); cout_endl();


#if 0
    v_w=300;
    v_h=400;
    fb_set_virtual_buffer_size( &v_w, &v_h );
    if( fb_get_virtual_buffer_size( &v_w, &v_h ) != FB_SUCCESS )
    {
        cout("fb_get_virtual_buffer_size error");cout_endl();
    }
    cout("virtual fb size: "); cout_d(v_w); cout("x"); cout_d(v_h); cout_endl();
#endif

    usleep(10000);
    gfx_set_env( p_fb, v_w, v_h, pitch, fbsize ); 
    gfx_set_bg( 0 );
    gfx_set_fg( 11 );
    gfx_clear();
}

void video_test()
{
    //gfx_fill_rect( 100, 100, 8, 8);
    //
    unsigned char ch='A';
    unsigned int row=0;
    unsigned int col=0;
    unsigned int term_cols, term_rows;
    gfx_get_term_size( &term_rows, &term_cols );

    unsigned int t0 = time_microsec();
    for( row=0; row<1000000; ++row )
    {
        gfx_putc(0,col,ch);
    }
    t0 = time_microsec()-t0;
    cout("T: ");cout_d(t0);cout_endl();
    return;


    while(1)
    {
        gfx_putc(row,col,ch);
        col = col+1;
        if( col >= term_cols )
        {

            col=0;
            row++;
            if( row > term_rows )
            {
                row=0;
                gfx_clear();
            }

        }
        ++ch;
        gfx_set_fg( ch );
        usleep(1000);
    }

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



void entry_point()
{
    uart_init();
    heartbeat_init();
    //heartbeat_loop();
    initialize_framebuffer();
    video_test();
}
