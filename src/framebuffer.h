//
// framebuffer.h
// Handle framebuffer and palette
//
// PiGFX is a bare metal kernel for the Raspberry Pi
// that implements a basic ANSI terminal emulator with
// the additional support of some primitive graphics functions.
// Copyright (C) 2014-2020 Filippo Bergamasco, Christian Lehner

#ifndef _FRAMEBUFFER_H_
#define _FRAMEBUFFER_H_



typedef enum {
FB_SUCCESS                = 0x0,
FB_ERROR                  = 0x1,
FB_POSTMAN_FAIL           = 0x2,
FB_GET_DISPLAY_SIZE_FAIL  = 0x3,
FB_FRAMEBUFFER_SETUP_FAIL = 0x4,
FB_INVALID_TAG_DATA       = 0x5,
FB_INVALID_PITCH          = 0x6
} FB_RETURN_TYPE;



extern FB_RETURN_TYPE fb_init( unsigned int ph_w, unsigned int ph_h, unsigned int vrt_w, unsigned int vrt_h,
                               unsigned int bpp, void** pp_fb, unsigned int* pfbsize, unsigned int* pPitch );
extern FB_RETURN_TYPE fb_release();
FB_RETURN_TYPE fb_get_phys_res(unsigned int* pRes_w, unsigned int* pRes_h);
extern FB_RETURN_TYPE fb_set_palette(unsigned char idx);
FB_RETURN_TYPE fb_get_pitch( unsigned int* pPitch );
FB_RETURN_TYPE fb_switch_framebuffer(unsigned int yOffset);
extern unsigned int* fb_get_cust_pal_p();


#endif
