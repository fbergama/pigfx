//
// framebuffer.c
// Handle framebuffer and palette
//
// PiGFX is a bare metal kernel for the Raspberry Pi
// that implements a basic ANSI terminal emulator with
// the additional support of some primitive graphics functions.
// Copyright (C) 2014-2020 Filippo Bergamasco, Christian Lehner

#include "pigfx_config.h"
#include "framebuffer.h"
#include "console.h"
#include "utils.h"
#include "synchronize.h"
#include "memory.h"
#include "mbox.h"
#include "palette.h"

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

    message_t* msg = (message_t*)MEM_COHERENT_REGION;

    msg->header.size = sizeof(*msg);
    msg->header.code = 0;

    msg->tag_phys_size.id = MAILBOX_TAG_SET_PHYSICAL_WIDTH_HEIGHT; // Set physical resolution
    msg->tag_phys_size.size = sizeof(msg->value_phys_size);
    msg->tag_phys_size.code = 0;
    msg->value_phys_size.request.display_w = ph_w;
    msg->value_phys_size.request.display_h = ph_h;

    msg->tag_virt_size.id = MAILBOX_TAG_SET_VIRTUAL_WIDTH_HEIGHT; // Set virtual resolution
    msg->tag_virt_size.size = sizeof(msg->value_virt_size);
    msg->tag_virt_size.code = 0;
    msg->value_virt_size.request.display_w = vrt_w;
    msg->value_virt_size.request.display_h = vrt_h*2;   // our virtual display is double the y size for implementing double buffering

    msg->tag_colour_depth.id = MAILBOX_TAG_SET_COLOUR_DEPTH; // Set colour depth
    msg->tag_colour_depth.size = sizeof(msg->value_colour_depth);
    msg->tag_colour_depth.code = 0;
    msg->value_colour_depth.request.depth = bpp;

    msg->tag_get_buf.id = MAILBOX_TAG_ALLOCATE_FRAMEBUFFER; // we want one
    msg->tag_get_buf.size = sizeof(msg->value_get_buf);
    msg->tag_get_buf.code = 0;
    msg->value_get_buf.request.alignment = 16;

    msg->footer.end = 0;

    CleanDataCache ();
    DataSyncBarrier ();
    if (mbox_send(msg) != 0) {
        return FB_ERROR;
    }
    InvalidateDataCache ();

    if ((msg->value_get_buf.response.bufferaddr == 0) || (msg->value_get_buf.response.buffersize == 0)) return FB_INVALID_TAG_DATA;

    /* physical_screenbase is the address of the screen in RAM
    *   * screenbase needs to be the screen address in virtual memory
    *       */
    *pp_fb = (void*)mem_vc2arm(msg->value_get_buf.response.bufferaddr);
    *pfbsize = msg->value_get_buf.response.buffersize;

    // Get pitch (bytes per line)
    if (fb_get_pitch(pPitch) != 0) return FB_INVALID_PITCH;

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

    message_t* msg = (message_t*)MEM_COHERENT_REGION;

    msg->header.size = sizeof(*msg);
    msg->header.code = 0;
    msg->tag.id = MAILBOX_TAG_RELEASE_FRAMEBUFFER; // Release framebuffer
    msg->tag.size = 0;
    msg->tag.code = 0;
    msg->footer.end = 0;

    if (mbox_send(msg) != 0) {
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

    message_t* msg = (message_t*)MEM_COHERENT_REGION;

    msg->header.size = sizeof(*msg);
    msg->header.code = 0;
    msg->tag.id = MAILBOX_TAG_GET_PHYSICAL_WIDTH_HEIGHT; // Get physical resolution
    msg->tag.size = sizeof(msg->value);
    msg->tag.code = 0;
    msg->footer.end = 0;

    if (mbox_send(msg) != 0) {
        return FB_ERROR;
    }

    *pRes_w = msg->value.response.display_w;
    *pRes_h = msg->value.response.display_h;

    return FB_SUCCESS;
}

FB_RETURN_TYPE fb_set_palette(unsigned char idx)
{
    // check idx
    idx = idx % NB_PALETTES;

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

    message_t* msg = (message_t*)MEM_COHERENT_REGION;

    msg->header.size = sizeof(*msg);
    msg->header.code = 0;
    msg->tag.id = MAILBOX_TAG_SET_PALETTE;
    msg->tag.size = sizeof(msg->value);
    msg->tag.code = 0;

    msg->value.request.offset = 0;
    msg->value.request.nbrOfEntries = NB_PALETTE_ELE;
    for(i=0;i<NB_PALETTE_ELE;i++)
    {
        const unsigned int vc = palette[idx][i];
        msg->value.request.entries[i] = (vc<<16 & 0xFF0000) | ( vc & 0x00FF00) | ( vc>>16 & 0x0000FF) | 0xFF000000;
    }

    msg->footer.end = 0;

    if (mbox_send(msg) != 0) {
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

    message_t* msg = (message_t*)MEM_COHERENT_REGION;

    msg->header.size = sizeof(*msg);
    msg->header.code = 0;
    msg->tag.id = MAILBOX_TAG_GET_PITCH; // Get pitch (bytes per line)
    msg->tag.size = sizeof(msg->value);
    msg->tag.code = 0;
    msg->footer.end = 0;

    if (mbox_send(msg) != 0) {
        return FB_ERROR;
    }

    *pPitch = msg->value.response.pitch;

    return FB_SUCCESS;
}

FB_RETURN_TYPE fb_switch_framebuffer(unsigned int yOffset)
{
    typedef struct {
        mbox_msgheader_t header;
        mbox_tagheader_t tag;

        union {
            struct {
                uint32_t xOffset;
                uint32_t yOffset;
            }
            request;
            // No response.
        }
        value;

        mbox_msgfooter_t footer;
    }
    message_t;

    message_t* msg = (message_t*)MEM_COHERENT_REGION;

    msg->header.size = sizeof(*msg);
    msg->header.code = 0;
    msg->tag.id = MAILBOX_TAG_SET_VIRTUAL_OFFSET;
    msg->tag.size = sizeof(msg->value);
    msg->tag.code = 0;
    msg->value.request.xOffset = 0;
    msg->value.request.yOffset = yOffset;
    msg->footer.end = 0;

    if (mbox_send(msg) != 0) {
        return FB_ERROR;
    }

    return FB_SUCCESS;
}

unsigned int* fb_get_cust_pal_p()
{
    return &palette[pal_custom][0];
}
