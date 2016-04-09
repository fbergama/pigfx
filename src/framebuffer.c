
#include "framebuffer.h"
#include "postman.h"
#include "console.H"
#include "utils.h"


static const unsigned int xterm_colors[256] = {
0x000000,  0x800000,  0x008000,  0x808000,  0x000080,
0x800080,  0x008080,  0xc0c0c0,  0x808080,  0xff0000,
0x00ff00,  0xffff00,  0x0000ff,  0xff00ff,  0x00ffff,
0xffffff,  0x000000,  0x00005f,  0x000087,  0x0000af,
0x0000df,  0x0000ff,  0x005f00,  0x005f5f,  0x005f87,
0x005faf,  0x005fdf,  0x005fff,  0x008700,  0x00875f,
0x008787,  0x0087af,  0x0087df,  0x0087ff,  0x00af00,
0x00af5f,  0x00af87,  0x00afaf,  0x00afdf,  0x00afff,
0x00df00,  0x00df5f,  0x00df87,  0x00dfaf,  0x00dfdf,
0x00dfff,  0x00ff00,  0x00ff5f,  0x00ff87,  0x00ffaf,
0x00ffdf,  0x00ffff,  0x5f0000,  0x5f005f,  0x5f0087,
0x5f00af,  0x5f00df,  0x5f00ff,  0x5f5f00,  0x5f5f5f,
0x5f5f87,  0x5f5faf,  0x5f5fdf,  0x5f5fff,  0x5f8700,
0x5f875f,  0x5f8787,  0x5f87af,  0x5f87df,  0x5f87ff,
0x5faf00,  0x5faf5f,  0x5faf87,  0x5fafaf,  0x5fafdf,
0x5fafff,  0x5fdf00,  0x5fdf5f,  0x5fdf87,  0x5fdfaf,
0x5fdfdf,  0x5fdfff,  0x5fff00,  0x5fff5f,  0x5fff87,
0x5fffaf,  0x5fffdf,  0x5fffff,  0x870000,  0x87005f,
0x870087,  0x8700af,  0x8700df,  0x8700ff,  0x875f00,
0x875f5f,  0x875f87,  0x875faf,  0x875fdf,  0x875fff,
0x878700,  0x87875f,  0x878787,  0x8787af,  0x8787df,
0x8787ff,  0x87af00,  0x87af5f,  0x87af87,  0x87afaf,
0x87afdf,  0x87afff,  0x87df00,  0x87df5f,  0x87df87,
0x87dfaf,  0x87dfdf,  0x87dfff,  0x87ff00,  0x87ff5f,
0x87ff87,  0x87ffaf,  0x87ffdf,  0x87ffff,  0xaf0000,
0xaf005f,  0xaf0087,  0xaf00af,  0xaf00df,  0xaf00ff,
0xaf5f00,  0xaf5f5f,  0xaf5f87,  0xaf5faf,  0xaf5fdf,
0xaf5fff,  0xaf8700,  0xaf875f,  0xaf8787,  0xaf87af,
0xaf87df,  0xaf87ff,  0xafaf00,  0xafaf5f,  0xafaf87,
0xafafaf,  0xafafdf,  0xafafff,  0xafdf00,  0xafdf5f,
0xafdf87,  0xafdfaf,  0xafdfdf,  0xafdfff,  0xafff00,
0xafff5f,  0xafff87,  0xafffaf,  0xafffdf,  0xafffff,
0xdf0000,  0xdf005f,  0xdf0087,  0xdf00af,  0xdf00df,
0xdf00ff,  0xdf5f00,  0xdf5f5f,  0xdf5f87,  0xdf5faf,
0xdf5fdf,  0xdf5fff,  0xdf8700,  0xdf875f,  0xdf8787,
0xdf87af,  0xdf87df,  0xdf87ff,  0xdfaf00,  0xdfaf5f,
0xdfaf87,  0xdfafaf,  0xdfafdf,  0xdfafff,  0xdfdf00,
0xdfdf5f,  0xdfdf87,  0xdfdfaf,  0xdfdfdf,  0xdfdfff,
0xdfff00,  0xdfff5f,  0xdfff87,  0xdfffaf,  0xdfffdf,
0xdfffff,  0xff0000,  0xff005f,  0xff0087,  0xff00af,
0xff00df,  0xff00ff,  0xff5f00,  0xff5f5f,  0xff5f87,
0xff5faf,  0xff5fdf,  0xff5fff,  0xff8700,  0xff875f,
0xff8787,  0xff87af,  0xff87df,  0xff87ff,  0xffaf00,
0xffaf5f,  0xffaf87,  0xffafaf,  0xffafdf,  0xffafff,
0xffdf00,  0xffdf5f,  0xffdf87,  0xffdfaf,  0xffdfdf,
0xffdfff,  0xffff00,  0xffff5f,  0xffff87,  0xffffaf,
0xffffdf,  0xffffff,  0x080808,  0x121212,  0x1c1c1c,
0x262626,  0x303030,  0x3a3a3a,  0x444444,  0x4e4e4e,
0x585858,  0x606060,  0x666666,  0x767676,  0x808080,
0x8a8a8a,  0x949494,  0x9e9e9e,  0xa8a8a8,  0xb2b2b2,
0xbcbcbc,  0xc6c6c6,  0xd0d0d0,  0xdadada,  0xe4e4e4,
0xeeeeee 
};



/*
 * Framebuffer initialization is a modifided version 
 * of the code originally written by brianwiddas.
 *
 * See:
 * https://github.com/brianwiddas/pi-baremetal.git
 *
 */
FB_RETURN_TYPE fb_init( unsigned int ph_w, unsigned int ph_h, unsigned int vrt_w, unsigned int vrt_h,
                        unsigned int bpp, void** pp_fb, unsigned int* pfbsize, unsigned int* pPitch )
{
    unsigned int var;
    unsigned int count;
    unsigned int physical_screenbase;

    /* Storage space for the buffer used to pass information between the
     * CPU and VideoCore
     * Needs to be aligned to 16 bytes as the bottom 4 bits of the address
     * passed to VideoCore are used for the mailbox number
     */
    volatile unsigned int mailbuffer[256] __attribute__((aligned (16)));

    /* Physical memory address of the mailbuffer, for passing to VC */
    unsigned int physical_mb = mem_v2p((unsigned int)mailbuffer);

    /* Get the display size */
    mailbuffer[0] = 8 * 4;      // Total size
    mailbuffer[1] = 0;          // Request
    mailbuffer[2] = 0x40003;    // Display size
    mailbuffer[3] = 8;          // Buffer size
    mailbuffer[4] = 0;          // Request size
    mailbuffer[5] = 0;          // Space for horizontal resolution
    mailbuffer[6] = 0;          // Space for vertical resolution
    mailbuffer[7] = 0;          // End tag

    if( POSTMAN_SUCCESS != postman_send( 8, physical_mb ) )
        return FB_POSTMAN_FAIL;

    if( POSTMAN_SUCCESS != postman_recv( 8, &var ) )
        return FB_POSTMAN_FAIL;

    /* Valid response in data structure */
    if(mailbuffer[1] != 0x80000000)
        return FB_GET_DISPLAY_SIZE_FAIL;


#ifdef FRAMEBUFFER_DEBUG
zsh:1: command not found: xclip
    unsigned int display_w = mailbuffer[5];
    unsigned int display_h = mailbuffer[6];
    cout("Display size: ");cout_d(display_w);cout("x");cout_d(display_h);cout_endl();
#endif

    /* Set up screen */
    unsigned int c = 1;
    mailbuffer[c++] = 0;        // Request

    mailbuffer[c++] = 0x00048003;   // Tag id (set physical size)
    mailbuffer[c++] = 8;        // Value buffer size (bytes)
    mailbuffer[c++] = 8;        // Req. + value length (bytes)
    mailbuffer[c++] = ph_w;     // Horizontal resolution
    mailbuffer[c++] = ph_h;     // Vertical resolution

    mailbuffer[c++] = 0x00048004;   // Tag id (set virtual size)
    mailbuffer[c++] = 8;        // Value buffer size (bytes)
    mailbuffer[c++] = 8;        // Req. + value length (bytes)
    mailbuffer[c++] = vrt_w;     // Horizontal resolution
    mailbuffer[c++] = vrt_h;     // Vertical resolution

    mailbuffer[c++] = 0x00048005;   // Tag id (set depth)
    mailbuffer[c++] = 4;        // Value buffer size (bytes)
    mailbuffer[c++] = 4;        // Req. + value length (bytes)
    mailbuffer[c++] = bpp;      //  bpp

    mailbuffer[c++] = 0x00040001;   // Tag id (allocate framebuffer)
    mailbuffer[c++] = 8;        // Value buffer size (bytes)
    mailbuffer[c++] = 4;        // Req. + value length (bytes)
    mailbuffer[c++] = 16;       // Alignment = 16
    mailbuffer[c++] = 0;        // Space for response

    mailbuffer[c++] = 0;        // Terminating tag

    mailbuffer[0] = c*4;        // Buffer size


    if( POSTMAN_SUCCESS != postman_send( 8, physical_mb ) )
        return FB_POSTMAN_FAIL;

    if( POSTMAN_SUCCESS != postman_recv( 8, &var ) )
        return FB_POSTMAN_FAIL;


    /* Valid response in data structure */
    if(mailbuffer[1] != 0x80000000)
        return FB_FRAMEBUFFER_SETUP_FAIL;

    count=2;    /* First tag */
    while((var = mailbuffer[count]))
    {
        if(var == 0x40001)
            break;

        /* Skip to next tag
         * Advance count by 1 (tag) + 2 (buffer size/value size)
         *                          + specified buffer size
         */
        count += 3+(mailbuffer[count+1]>>2);

        if(count>c)
            return FB_FRAMEBUFFER_SETUP_FAIL;
    }

    /* 8 bytes, plus MSB set to indicate a response */
    if(mailbuffer[count+2] != 0x80000008)
        return FB_FRAMEBUFFER_SETUP_FAIL;

    /* Framebuffer address/size in response */
    physical_screenbase = mailbuffer[count+3];
    *pfbsize = mailbuffer[count+4];

    if(physical_screenbase == 0 || *pfbsize == 0)
        return FB_INVALID_TAG_DATA;

    /* physical_screenbase is the address of the screen in RAM
     *   * screenbase needs to be the screen address in virtual memory
     *       */
    *pp_fb = (void*)mem_p2v(physical_screenbase);

#ifdef FRAMEBUFFER_DEBUG
    cout("Screen addr: ");cout_h((unsigned int)*pp_fb); cout_endl();
    cout("Screen size: ");cout_d(*pfbsize); cout_endl();
#endif

    /* Get the framebuffer pitch (bytes per line) */
    mailbuffer[0] = 7 * 4;      // Total size
    mailbuffer[1] = 0;      // Request
    mailbuffer[2] = 0x40008;    // Display size
    mailbuffer[3] = 4;      // Buffer size
    mailbuffer[4] = 0;      // Request size
    mailbuffer[5] = 0;      // Space for pitch
    mailbuffer[6] = 0;      // End tag

    if( POSTMAN_SUCCESS != postman_send( 8, physical_mb ) )
        return FB_POSTMAN_FAIL;

    if( POSTMAN_SUCCESS != postman_recv( 8, &var ) )
        return FB_POSTMAN_FAIL;

    /* 4 bytes, plus MSB set to indicate a response */
    if(mailbuffer[4] != 0x80000004)
        return FB_INVALID_PITCH;

    *pPitch = mailbuffer[5];

    if( *pPitch == 0 )
        return FB_INVALID_PITCH;

#ifdef FRAMEBUFFER_DEBUG
    cout("pitch: "); cout_d(*pPitch); cout_endl();
#endif

    return FB_SUCCESS;
}



FB_RETURN_TYPE fb_release()
{
    volatile unsigned int pBuffData[256] __attribute__((aligned (16)));
    unsigned int off;
    unsigned int respmsg;


    off=1;
    pBuffData[off++] = 0;           // Request 
    pBuffData[off++] = 0x00048001;  // Tag: blank
    pBuffData[off++] = 0;           // response buffer size in bytes
    pBuffData[off++] = 0;           // request size
    pBuffData[off++] = 0;           // end tag

    pBuffData[0]=off*4; // Total message size

    unsigned int physical_mb = mem_v2p((unsigned int)pBuffData);
    if( POSTMAN_SUCCESS != postman_send( 8, physical_mb ) )
        return FB_POSTMAN_FAIL;

    if( POSTMAN_SUCCESS != postman_recv( 8, &respmsg ) )
        return FB_POSTMAN_FAIL;

    if( pBuffData[1]!=0x80000000 )
    {
        return FB_ERROR;
    }

    return FB_SUCCESS;

}


FB_RETURN_TYPE fb_set_grayscale_palette()
{
    volatile unsigned int pBuffData[4096] __attribute__((aligned (16)));
    unsigned int off;
    unsigned int respmsg;

    off=1;
    pBuffData[off++] = 0;           // Request 
    pBuffData[off++] = 0x0004800B;  // Tag: blank
    pBuffData[off++] = 4;           // response buffer size in bytes
    pBuffData[off++] = 1032;        // request size
    pBuffData[off++] = 0;           // first palette index
    pBuffData[off++] = 256;         // num entries 

    unsigned int pi;
    for(pi=0;pi<256;++pi)
    {
        unsigned int entry = 0;

        entry =  (pi & 0xFF)<<16 | (pi & 0xFF)<<8 | (pi & 0xFF);
        entry = entry | 0xFF000000; //alpha

        pBuffData[off++] = entry;         
    }
    pBuffData[off++] = 0;           // end tag

    pBuffData[0]=off*4; // Total message size

    unsigned int physical_mb = mem_v2p((unsigned int)pBuffData);
    if( POSTMAN_SUCCESS != postman_send( 8, physical_mb ) )
        return FB_POSTMAN_FAIL;

    if( POSTMAN_SUCCESS != postman_recv( 8, &respmsg ) )
        return FB_POSTMAN_FAIL;

    if( pBuffData[1]!=0x80000000 )
    {
        return FB_ERROR;
    }

    return FB_SUCCESS;

}


FB_RETURN_TYPE fb_set_xterm_palette()
{
    volatile unsigned int pBuffData[4096] __attribute__((aligned (16)));
    unsigned int off;
    unsigned int respmsg;

    off=1;
    pBuffData[off++] = 0;           // Request 
    pBuffData[off++] = 0x0004800B;  // Tag: blank
    pBuffData[off++] = 4;           // response buffer size in bytes
    pBuffData[off++] = 1032;        // request size
    pBuffData[off++] = 0;           // first palette index
    pBuffData[off++] = 256;         // num entries 

    unsigned int pi;
    for(pi=0;pi<256;++pi)
    {
        const unsigned int vc = xterm_colors[pi];
        // RGB -> BGR
        pBuffData[off++] = (vc<<16 & 0xFF0000) | ( vc & 0x00FF00) | ( vc>>16 & 0x0000FF) | 0xFF000000;         
    }
    pBuffData[off++] = 0;           // end tag

    pBuffData[0]=off*4; // Total message size

    unsigned int physical_mb = mem_v2p((unsigned int)pBuffData);
    if( POSTMAN_SUCCESS != postman_send( 8, physical_mb ) )
        return FB_POSTMAN_FAIL;

    if( POSTMAN_SUCCESS != postman_recv( 8, &respmsg ) )
        return FB_POSTMAN_FAIL;

    if( pBuffData[1]!=0x80000000 )
    {
        return FB_ERROR;
    }

    return FB_SUCCESS;
}


FB_RETURN_TYPE fb_blank_screen( unsigned int blank )
{
    volatile unsigned int pBuffData[256] __attribute__((aligned (16)));
    unsigned int off;
    unsigned int respmsg;


    off=1;
    pBuffData[off++] = 0;           // Request 
    pBuffData[off++] = 0x00040002;  // Tag: blank
    pBuffData[off++] = 4;           // response buffer size in bytes
    pBuffData[off++] = 4;           // request size
    pBuffData[off++] = blank;       // state
    pBuffData[off++] = 0;           // end tag

    pBuffData[0]=off*4; // Total message size

    unsigned int physical_mb = mem_v2p((unsigned int)pBuffData);
    if( POSTMAN_SUCCESS != postman_send( 8, physical_mb ) )
        return FB_POSTMAN_FAIL;

    if( POSTMAN_SUCCESS != postman_recv( 8, &respmsg ) )
        return FB_POSTMAN_FAIL;

    if( pBuffData[1]!=0x80000000 )
    {
        return FB_ERROR;
    }

    return FB_SUCCESS;
}



FB_RETURN_TYPE fb_set_depth( unsigned int* pDepth )
{
    volatile unsigned int pBuffData[256] __attribute__((aligned (16)));
    unsigned int off;
    unsigned int respmsg;

    off=1;
    pBuffData[off++] = 0;           // Request 
    pBuffData[off++] = 0x00048005;  // Tag: get pitch 
    pBuffData[off++] = 4;           // response buffer size in bytes
    pBuffData[off++] = 4;           // request size 
    pBuffData[off++] = *pDepth;           // response buffer
    pBuffData[off++] = 0;           // end tag

    pBuffData[0]=off*4; // Total message size

    postman_send( 8, mem_v2p((unsigned int)pBuffData) );
    postman_recv( 8, &respmsg);

    if( pBuffData[1]!=0x80000000 )
    {
        return FB_ERROR;
    }

    *pDepth = pBuffData[5];

    return FB_SUCCESS;
}



FB_RETURN_TYPE fb_get_pitch( unsigned int* pPitch )
{
    volatile unsigned int pBuffData[256] __attribute__((aligned (16)));
    unsigned int off;
    unsigned int respmsg;

    off=1;
    pBuffData[off++] = 0;           // Request 
    pBuffData[off++] = 0x00040008;  // Tag: get pitch 
    pBuffData[off++] = 4;           // response buffer size in bytes
    pBuffData[off++] = 0;           // request size 
    pBuffData[off++] = 0;           // response buffer
    pBuffData[off++] = 0;           // end tag

    pBuffData[0]=off*4; // Total message size

    postman_send( 8, mem_v2p((unsigned int)pBuffData) );
    postman_recv( 8, &respmsg);

    if( pBuffData[1]!=0x80000000 )
    {
        return FB_ERROR;
    }

    if( pPitch )
        *pPitch = pBuffData[5];

    return FB_SUCCESS;
}


FB_RETURN_TYPE fb_get_phisical_buffer_size( unsigned int* pWidth, unsigned int* pHeight )
{
    volatile unsigned int pBuffData[256] __attribute__((aligned (16)));
    unsigned int off;
    unsigned int respmsg;

    off=1;
    pBuffData[off++] = 0;           // Request 
    pBuffData[off++] = 0x00040003;  // Tag: get phisical buffer size 
    pBuffData[off++] = 8;           // response buffer size in bytes
    pBuffData[off++] = 0;           // request size 
    pBuffData[off++] = 0;           // response buffer
    pBuffData[off++] = 0;           // response buffer
    pBuffData[off++] = 0;           // end tag

    pBuffData[0]=off*4; // Total message size

    postman_send( 8, mem_v2p((unsigned int)pBuffData) );
    postman_recv( 8, &respmsg);

    if( pBuffData[1]!=0x80000000 )
    {
        return FB_ERROR;
    }

    if( pWidth )
        *pWidth = pBuffData[5];
    if( pHeight )
        *pHeight = pBuffData[6];

    return FB_SUCCESS;
}


FB_RETURN_TYPE fb_set_phisical_buffer_size( unsigned int* pWidth, unsigned int* pHeight )
{
    volatile unsigned int pBuffData[256] __attribute__((aligned (16)));
    unsigned int off;
    unsigned int respmsg;

    off=1;
    pBuffData[off++] = 0;           // Request 
    pBuffData[off++] = 0x00048003;  // Tag: set phisical buffer size 
    pBuffData[off++] = 8;           // response buffer size in bytes
    pBuffData[off++] = 8;           // request size 
    pBuffData[off++] = *pWidth;     // response buffer
    pBuffData[off++] = *pHeight;    // response buffer
    pBuffData[off++] = 0;           // end tag

    pBuffData[0]=off*4; // Total message size

    postman_send( 8, mem_v2p((unsigned int)pBuffData) );
    postman_recv( 8, &respmsg);

    if( pBuffData[1]!=0x80000000 )
    {
        return FB_ERROR;
    }

    if( pWidth )
        *pWidth = pBuffData[5];
    if( pHeight )
        *pHeight = pBuffData[6];

    return FB_SUCCESS;
}


FB_RETURN_TYPE fb_allocate_buffer( void** ppBuffer, unsigned int* pBufferSize )
{
    volatile unsigned int pBuffData[256] __attribute__((aligned (16)));
    unsigned int off;
    unsigned int respmsg;

    off=1;
    pBuffData[off++] = 0;           // Request 
    pBuffData[off++] = 0x00040004;  // Tag: allocate buffer 
    pBuffData[off++] = 8;           // response buffer size in bytes
    pBuffData[off++] = 4;           // request size 
    pBuffData[off++] = 16;          // response buffer
    pBuffData[off++] = 0;           // response buffer
    pBuffData[off++] = 0;           // end tag

    pBuffData[0]=off*4; // Total message size

    postman_send( 8, mem_v2p((unsigned int)pBuffData) );
    postman_recv( 8, &respmsg);

    if( pBuffData[1]!=0x80000000 )
    {
        return FB_ERROR;
    }

    *ppBuffer = (void*)pBuffData[5];
    *pBufferSize = pBuffData[6];

    return FB_SUCCESS;
}


FB_RETURN_TYPE fb_get_virtual_buffer_size( unsigned int* pVWidth, unsigned int* pVHeight )
{
    volatile unsigned int pBuffData[256] __attribute__((aligned (16)));
    unsigned int off;
    unsigned int respmsg;

    off=1;
    pBuffData[off++] = 0;           // Request 
    pBuffData[off++] = 0x00040004;  // Tag: get virtual buffer size 
    pBuffData[off++] = 8;           // response buffer size in bytes
    pBuffData[off++] = 0;           // request size 
    pBuffData[off++] = 0;           // response buffer
    pBuffData[off++] = 0;           // response buffer
    pBuffData[off++] = 0;           // end tag

    pBuffData[0]=off*4; // Total message size

    postman_send( 8, mem_v2p((unsigned int)pBuffData) );
    postman_recv( 8, &respmsg);

    if( pBuffData[1]!=0x80000000 )
    {
        return FB_ERROR;
    }

    if( pVWidth )
        *pVWidth = pBuffData[5];
    if( pVHeight )
        *pVHeight = pBuffData[6];

    return FB_SUCCESS;
}


FB_RETURN_TYPE fb_set_virtual_buffer_size( unsigned int* pWidth, unsigned int* pHeight )
{
    volatile unsigned int pBuffData[256] __attribute__((aligned (16)));
    unsigned int off;
    unsigned int respmsg;

    off=1;
    pBuffData[off++] = 0;           // Request 
    pBuffData[off++] = 0x00048004;  // Tag: set virtual buffer size 
    pBuffData[off++] = 8;           // response buffer size in bytes
    pBuffData[off++] = 8;           // request size 
    pBuffData[off++] = *pWidth;       // response buffer
    pBuffData[off++] = *pHeight;      // response buffer
    pBuffData[off++] = 0;           // end tag

    pBuffData[0]=off*4; // Total message size

    postman_send( 8, mem_v2p((unsigned int)pBuffData) );
    postman_recv( 8, &respmsg);

    if( pBuffData[1]!=0x80000000 )
    {
        return FB_ERROR;
    }

    if( pWidth )
        *pWidth = pBuffData[5];
    if( pHeight )
        *pHeight = pBuffData[6];

    return FB_SUCCESS;
}


FB_RETURN_TYPE fb_set_virtual_offset( unsigned int pX, unsigned int pY )
{
    volatile unsigned int pBuffData[256] __attribute__((aligned (16)));
    unsigned int off;
    unsigned int respmsg;

    off=1;
    pBuffData[off++] = 0;           // Request 
    pBuffData[off++] = 0x00048009;  // Tag: set virtual offset 
    pBuffData[off++] = 8;           // response buffer size in bytes
    pBuffData[off++] = 8;           // request size 
    pBuffData[off++] = pX;           // response buffer
    pBuffData[off++] = pY;           // response buffer
    pBuffData[off++] = 0;           // end tag

    pBuffData[0]=off*4; // Total message size

    postman_send( 8, mem_v2p((unsigned int)pBuffData) );
    postman_recv( 8, &respmsg);

    if( pBuffData[1]!=0x80000000 )
    {
        return FB_ERROR;
    }

    return FB_SUCCESS;
}
