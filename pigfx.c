#include "uart.h"
#include "utils.h"
#include "timer.h"
#include "framebuffer.h"
#include "console.h"

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


void video_test()
{
    uart_write_str("Initializing video..");
    usleep(1000000);

#if 1

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
    if( fb_get_virtual_buffer_size( &v_w, &v_h ) != FB_SUCCESS )
    {
        cout("fb_get_virtual_buffer_size error");cout_endl();
    }
    cout("virtual fb size: "); cout_d(v_w); cout("x"); cout_d(v_h); cout_endl();

    usleep(10000);

    
    unsigned int n_frames = 0;
    unsigned int RR=0;
    while(1)
    {
        unsigned int row;

        for( row=0; row<v_h;++row)
        {
            unsigned char* pVid =  p_fb + pitch*row ;
            const unsigned char* pVidEnd = pVid + v_w;
            const unsigned char val = RR % 256;
            while(pVid<pVidEnd)
            {
                *pVid++=val;
            }
        } 
        RR = (RR+1) % 16;
        ++n_frames;
        usleep(1000000);
    }
#endif

}



void entry_point()
{
    uart_init();
    heartbeat_init();
    //heartbeat_loop();
    video_test();
}
