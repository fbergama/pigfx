#include "pigfx_config.h"
#include "framebuffer.h"
#include "console.h"
#include "utils.h"
#include "mbox.h"

#define NB_PALETTE_ELE 256


static const unsigned int xterm_colors[NB_PALETTE_ELE] = {
// 16 half/full bright RGB colors
0x000000,  0x800000,  0x008000,  0x808000,  0x000080,
0x800080,  0x008080,  0xc0c0c0,  0x808080,  0xff0000,
0x00ff00,  0xffff00,  0x0000ff,  0xff00ff,  0x00ffff,
0xffffff,
// gradients
0x000000,  0x00005f,  0x000087,  0x0000af,
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
0xffffdf,  0xffffff,
// 24 gray scales
0x080808,  0x121212,  0x1c1c1c,
0x262626,  0x303030,  0x3a3a3a,  0x444444,  0x4e4e4e,
0x585858,  0x606060,  0x666666,  0x767676,  0x808080,
0x8a8a8a,  0x949494,  0x9e9e9e,  0xa8a8a8,  0xb2b2b2,
0xbcbcbc,  0xc6c6c6,  0xd0d0d0,  0xdadada,  0xe4e4e4,
0xeeeeee 
};



/*
 * Framebuffer initialization is a modified version
 * of the code originally written by brianwiddas.
 *
 * See:
 * https://github.com/brianwiddas/pi-baremetal.git
 *
 * Now it's even more modified by Christian Lehner
 */
FB_RETURN_TYPE fb_init( unsigned int ph_w, unsigned int ph_h, unsigned int vrt_w, unsigned int vrt_h,
                        unsigned int bpp, void** pp_fb, unsigned int* pfbsize, unsigned int* pPitch )
{
#if ENABLED(FRAMEBUFFER_DEBUG)
    unsigned int getPhysRes_w, getPhysRes_h;
    
    // Get physical display size
    if (fb_get_phys_res(&getPhysRes_w, &getPhysRes_h) != 0) return FB_GET_DISPLAY_SIZE_FAIL;
    cout("Display size: ");cout_d(getPhysRes_w);cout("x");cout_d(getPhysRes_h);cout_endl();
#endif

    // Set up the framebuffer and get one
    // Set display size
    typedef struct
    {
        mbox_msgheader_t header;
        
        mbox_tagheader_t tag_phys_size;
        union
        {
            struct
            {
                uint32_t display_w;
                uint32_t display_h;
            }
            request;
            struct
            {
                uint32_t act_display_w;
                uint32_t act_display_h;
            }
            response;
        }
        value_phys_size;
        
        mbox_tagheader_t tag_virt_size;
        union
        {
            struct
            {
                uint32_t display_w;
                uint32_t display_h;
            }
            request;
            struct
            {
                uint32_t act_display_w;
                uint32_t act_display_h;
            }
            response;
        }
        value_virt_size;
        
        mbox_tagheader_t tag_colour_depth;
        union
        {
            struct
            {
                uint32_t depth;
            }
            request;
            struct
            {
                uint32_t act_depth;
            }
            response;
        }
        value_colour_depth;
        
        mbox_tagheader_t tag_get_buf;
        union
        {
            struct
            {
                uint32_t alignment;
            }
            request;
            struct
            {
                uint32_t bufferaddr;
                uint32_t buffersize;
            }
            response;
        }
        value_get_buf;

        mbox_msgfooter_t footer;
    }
    message_t;

    message_t msg __attribute__((aligned(16)));

    msg.header.size = sizeof(msg);
    msg.header.code = 0;
    
    msg.tag_phys_size.id = MAILBOX_TAG_SET_PHYSICAL_WIDTH_HEIGHT; // Set physical resolution
    msg.tag_phys_size.size = sizeof(msg.value_phys_size);
    msg.tag_phys_size.code = 0;
    msg.value_phys_size.request.display_w = ph_w;
    msg.value_phys_size.request.display_h = ph_h;
    
    msg.tag_virt_size.id = MAILBOX_TAG_SET_VIRTUAL_WIDTH_HEIGHT; // Set virtual resolution
    msg.tag_virt_size.size = sizeof(msg.value_virt_size);
    msg.tag_virt_size.code = 0;
    msg.value_virt_size.request.display_w = vrt_w;
    msg.value_virt_size.request.display_h = vrt_h;
    
    msg.tag_colour_depth.id = MAILBOX_TAG_SET_COLOUR_DEPTH; // Set colour depth
    msg.tag_colour_depth.size = sizeof(msg.value_colour_depth);
    msg.tag_colour_depth.code = 0;
    msg.value_colour_depth.request.depth = bpp;
    
    msg.tag_get_buf.id = MAILBOX_TAG_ALLOCATE_FRAMEBUFFER; // we want one
    msg.tag_get_buf.size = sizeof(msg.value_get_buf);
    msg.tag_get_buf.code = 0;
    msg.value_get_buf.request.alignment = 16;
    
    msg.footer.end = 0;
    
    if (mbox_send(&msg) != 0) {
        return FB_ERROR;
    }
    
    if ((msg.value_get_buf.response.bufferaddr == 0) || (msg.value_get_buf.response.buffersize == 0)) return FB_INVALID_TAG_DATA;
    
    /* physical_screenbase is the address of the screen in RAM
    *   * screenbase needs to be the screen address in virtual memory
    *       */
    *pp_fb = (void*)mem_vc2arm(msg.value_get_buf.response.bufferaddr);
    *pfbsize = msg.value_get_buf.response.buffersize;
    
#if ENABLED(FRAMEBUFFER_DEBUG)
    cout("Set physical display size: ");cout_d(ph_w);cout("x");cout_d(ph_h);cout_endl();
    cout("Response: ");cout_d(msg.value_phys_size.response.act_display_w);cout("x");cout_d(msg.value_phys_size.response.act_display_h);cout_endl();
    cout("Set virtual display size: ");cout_d(vrt_w);cout("x");cout_d(vrt_h);cout_endl();
    cout("Response: ");cout_d(msg.value_virt_size.response.act_display_w);cout("x");cout_d(msg.value_virt_size.response.act_display_h);cout_endl();
    cout("Set colour depth: ");cout_d(bpp);cout(" bpp");cout_endl();
    cout("Response: ");cout_d(msg.value_colour_depth.response.act_depth);cout(" bpp");cout_endl();
    cout("Screen addr: ");cout_h((unsigned int)*pp_fb); cout_endl();
    cout("Screen size: ");cout_d(*pfbsize); cout_endl();
#endif
    
    // Get pitch (bytes per line)
    if (fb_get_pitch(pPitch) != 0) return FB_INVALID_PITCH;

#if ENABLED(FRAMEBUFFER_DEBUG)
    cout("Pitch: "); cout_d(*pPitch); cout_endl();
#endif

    return FB_SUCCESS;
}

FB_RETURN_TYPE fb_release()
{
    typedef struct {
        mbox_msgheader_t header;
        mbox_tagheader_t tag;
        mbox_msgfooter_t footer;
    }
    message_t;
    
    message_t msg __attribute__((aligned(16)));
    
    msg.header.size = sizeof(msg);
    msg.header.code = 0;
    msg.tag.id = MAILBOX_TAG_RELEASE_FRAMEBUFFER; // Release framebuffer
    msg.tag.size = 0;
    msg.tag.code = 0;
    msg.footer.end = 0;

    if (mbox_send(&msg) != 0) {
        return FB_ERROR;
    }
    
    return FB_SUCCESS;
}

FB_RETURN_TYPE fb_get_phys_res(unsigned int* pRes_w, unsigned int* pRes_h)
{
    // Get physical display size
    typedef struct
    {
        mbox_msgheader_t header;
        mbox_tagheader_t tag;

        union
        {
            // No request.
            struct
            {
                uint32_t display_w;
                uint32_t display_h;
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
    msg.tag.id = MAILBOX_TAG_GET_PHYSICAL_WIDTH_HEIGHT; // Get physical resolution
    msg.tag.size = sizeof(msg.value);
    msg.tag.code = 0;
    msg.footer.end = 0;

    if (mbox_send(&msg) != 0) {
        return FB_ERROR;
    }
    
    *pRes_w = msg.value.response.display_w;
    *pRes_h = msg.value.response.display_h;
    
    return FB_SUCCESS;
}

FB_RETURN_TYPE fb_set_grayscale_palette()
{
    // Set grayscale palette
    unsigned int i;
    typedef struct
    {
        mbox_msgheader_t header;
        mbox_tagheader_t tag;

        union
        {
            struct
            {
                uint32_t offset;
                uint32_t nbrOfEntries;
                uint32_t entries[NB_PALETTE_ELE];
            }
            request;
            struct
            {
                uint32_t invalid;
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
    msg.tag.id = MAILBOX_TAG_SET_PALETTE;
    msg.tag.size = sizeof(msg.value);
    msg.tag.code = 0;
    
    msg.value.request.offset = 0;
    msg.value.request.nbrOfEntries = NB_PALETTE_ELE;
    for(i=0;i<NB_PALETTE_ELE;i++)
    {
        msg.value.request.entries[i] = (i & 0xFF)<<16 | (i & 0xFF)<<8 | (i & 0xFF);
        msg.value.request.entries[i] = msg.value.request.entries[i] | 0xFF000000; //alpha
    }
    
    msg.footer.end = 0;
    
    if (mbox_send(&msg) != 0) {
        return FB_ERROR;
    }
    
    return FB_SUCCESS;
}

FB_RETURN_TYPE fb_set_xterm_palette()
{
    // Set xterm palette
    unsigned int i;
    typedef struct
    {
        mbox_msgheader_t header;
        mbox_tagheader_t tag;

        union
        {
            struct
            {
                uint32_t offset;
                uint32_t nbrOfEntries;
                uint32_t entries[NB_PALETTE_ELE];
            }
            request;
            struct
            {
                uint32_t invalid;
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
    msg.tag.id = MAILBOX_TAG_SET_PALETTE;
    msg.tag.size = sizeof(msg.value);
    msg.tag.code = 0;
    
    msg.value.request.offset = 0;
    msg.value.request.nbrOfEntries = NB_PALETTE_ELE;
    for(i=0;i<NB_PALETTE_ELE;i++)
    {
        const unsigned int vc = xterm_colors[i];
        msg.value.request.entries[i] = (vc<<16 & 0xFF0000) | ( vc & 0x00FF00) | ( vc>>16 & 0x0000FF) | 0xFF000000;
    }
    
    msg.footer.end = 0;
    
    if (mbox_send(&msg) != 0) {
        return FB_ERROR;
    }
    
    return FB_SUCCESS;
}

FB_RETURN_TYPE fb_get_pitch( unsigned int* pPitch )
{
    // Get pitch
    typedef struct
    {
        mbox_msgheader_t header;
        mbox_tagheader_t tag;

        union
        {
            // No request.
            struct
            {
                uint32_t pitch;
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
    msg.tag.id = MAILBOX_TAG_GET_PITCH; // Get pitch (bytes per line)
    msg.tag.size = sizeof(msg.value);
    msg.tag.code = 0;
    msg.footer.end = 0;

    if (mbox_send(&msg) != 0) {
        return FB_ERROR;
    }
    
    *pPitch = msg.value.response.pitch;
    
    return FB_SUCCESS;
}

