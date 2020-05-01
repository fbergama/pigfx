#include "pigfx_config.h"
#include "scn_state.h"
#include "gfx.h"
#include "framebuffer.h"
#include "console.h"
#include "dma.h"
#include "utils.h"
#include "ee_printf.h"
#include <stdlib.h>

#define MIN( v1, v2 ) ( ((v1) < (v2)) ? (v1) : (v2))
#define MAX( v1, v2 ) ( ((v1) > (v2)) ? (v1) : (v2))
#define PFB( X, Y ) ( ctx.pfb + (Y) * ctx.Pitch + (X) )

void __swap__( int* a, int* b )
{
    int aux = *a;
    *a = *b;
    *b = aux;
}

int __abs__( int a )
{
    return a<0?-a:a;
}

/** Function type to compute a glyph address in a font. */
typedef unsigned char* font_fun(unsigned int c);

/** Display properties.
 *  Holds relevant properties for the display routines. Members in this
 *  structure are updated when setting display mode or font or by getting
 *  information from DMA controller.
 */
typedef struct {
	// Graphics variables
    unsigned int W;						/// Screen pixel width
    unsigned int H;						/// Screen pixel height
    unsigned int bpp;					/// Bits depth
    unsigned int Pitch;					/// Number of bytes for one line (TODO:)
    unsigned int size;					/// Number of bytes in the framebuffer
    unsigned char* pfb;					/// Framebuffer address
    DRAWING_MODE mode;					/// Drawing mode: normal, xor, transparent
    unsigned char transparentcolor;		/// For transparent drawing mode

    // Terminal variables
    struct
    {
    	// Current Font variables
    	unsigned char* FONT;			/// Points to font resource
    	unsigned int FONTWIDTH;			/// Pixel width for characters
    	unsigned int FONTHEIGHT;		/// Pixel height for characters
    	unsigned int FONTCHARBYTES;		/// Number of bytes for one char in font
    	unsigned int FONTWIDTH_INTS;	/// Number of 32-bits integers for font width (4 pixels / int)
    	unsigned int FONTWIDTH_REMAIN;	/// Number of bytes to add to ints (when fontwidth not a multiple of 4)
    	font_fun (*font_getglyph);		/// Function to get a glyph address for a character

    	// Current Character Display variables
        unsigned int WIDTH;				/// Terminal character width (W / font width)
        unsigned int HEIGHT;			/// Terminal character height (H / font height)
        unsigned int tab_pos;			/// 8 by default, tabulation position multiplicator
        unsigned int cursor_row;		/// Current row position (0-based)
        unsigned int cursor_col;		/// Current column position (0-based)
        unsigned int saved_cursor[2];	/// Saved cursor position
        char cursor_visible;			/// 0 if no visible cursor

        scn_state state;				/// Current scan state
    } term;

    GFX_COL bg;							/// Background characters color
    GFX_COL fg;							/// Foreground characters color
    unsigned int bg32;					/// Computed ctx.bg<<24 | ctx.bg<<16 | ctx.bg<<8 | ctx.bg;
    unsigned int fg32;					/// Computed ctx.fg<<24 | ctx.fg<<16 | ctx.fg<<8 | ctx.fg;

    unsigned char* cursor_buffer;		/// Saved content under current buffer position
    unsigned int cursor_buffer_size;	/// Byte size of this buffer
    unsigned int cursor_buffer_ready;	/// 0 if buffer is empty, 1 of it stores a content

} FRAMEBUFFER_CTX;

/** Modes for ESC[=<mode>h - PC ANSI.SYS legacy */
struct DISPLAY_MODE_DEFINITION
{
	unsigned int width;
	unsigned int height;
	unsigned int bpp; 		// NB: all are actually forced to 8 bpp
};

// Number of known modes
#define LAST_MODE_NUMBER 20

/**
 * Display Modes, as they were interpreted by ANSI.SYS on PC with a CGA, EGA or VGA card
 * There is a lot of work to be done :
 * - BPP is 8 for all so all modes are actually 256 colors/pixel and 1 byte per pixel
 * - invalid modes don't do anything
 * - mode 7 (line wrapping) is ignored
 * - only 320x200 and 640*480 seem to actually modify the resolution, other modes merely limit the area on screen.
 */
static struct DISPLAY_MODE_DEFINITION ALL_MODES[LAST_MODE_NUMBER + 1] = {

		// Resolution          Corresponding PC mode    PC card
		//---------------------------------------------------

		// Legacy CGA
		{320,200,8},		// 0: text mono  40 x 25    (CGA)
		{320,200,8},		// 1: text color 40 x 25    (CGA)
		{640,480,8},		// 2: text mono  80 x 25    (CGA)
		{640,480,8},		// 3: text color 80 x 25    (CGA)
		{320,200,8},		// 4: 320 x 200 4 colors    (CGA)
		{320,200,8},		// 5: 320 x 200 mono        (CGA)
		{640,200,8},		// 6: 640 x 200 mono        (CGA)

		// Special or non assigned
		{0,0,0},			// 7: enable line wrapping
		{0,0,0},			// 8:
		{0,0,0},			// 9:
		{0,0,0},			// 10:
		{0,0,0},			// 11:
		{0,0,0},			// 12:

		// Legacy EGA
		{320,200,8},		// 13: 320 x 200 16 colors  (EGA)
		{640,200,8},		// 14: 640 x 200 16 colors  (EGA)
		{640,350,8},		// 15: 640 x 350 mono       (EGA)
		{640,350,8},		// 16: 640 x 350 16 colors  (EGA)

		// Legacy VGA
		{640,480,8},		// 17: 640 x 480 mono       (VGA)
		{640,480,8},		// 18: 640 x 480 16 colors  (VGA)
		{320,200,8},		// 19: 320 x 200 256 colors (MCGA)

		{320,240,8},		// 20: 320 x 240 256 colors (Michael Abrash X-Mode)
};

/** Forward declaration for some state functions. */
state_fun state_fun_finalletter;
state_fun state_fun_read_digit;
state_fun state_fun_selectescape;
state_fun state_fun_waitquarebracket;
state_fun state_fun_normaltext;
state_fun state_fun_ignore_digit;

#include "framebuffer.h"

// Global static to store the screen variables.
/*static*/ FRAMEBUFFER_CTX ctx;

// DMA communication and command buffer
unsigned int __attribute__((aligned(0x100))) mem_buff_dma[16];

// local memset
void local_memset (void *pBuffer, int nValue, unsigned int nLength)
{
	char *p = (char *) pBuffer;

	while (nLength--)
	{
		*p++ = (char) nValue;
	}
}

// Forward declarations
void gfx_term_render_cursor();
void gfx_term_save_cursor_content();

// Functions from pigfx.c called by some private sequences (set mode, debug tests ...)
extern void initialize_framebuffer(unsigned int width, unsigned int height, unsigned int bpp);
//extern void video_line_test(int maxloops);
//extern void video_test(int maxloops);

/** Fonts from binary_assets.s and fonts (*.bin) */
extern unsigned char G_FONT8X8_GLYPHS;
extern unsigned char G_FONT8X16_GLYPHS;
extern unsigned char G_FONT8X24_GLYPHS;

/** Font function for the default 8x8 font. */
unsigned char* font_get_glyph_address_8x8(unsigned int c)
{
	// offset of glyph is c * 64 bytes, which can be computed by c << 6
	return ctx.term.FONT + ((unsigned int)c<<6);
}
/** Font function for the 8x24 font. */
unsigned char* font_get_glyph_address_8x24(unsigned int c)
{
	// offset of glyph is c * 192 bytes, which is c << 7 + c << 6
	//unsigned int c64 = (unsigned int)(c<<6);
	//return (unsigned int*)( ctx.term.FONT + c64 + (c64 << 1));
	return ctx.term.FONT + c * 192;
}
/** Font function for the 8x16 font. */
unsigned char* font_get_glyph_address_8x16(unsigned int c)
{
	// offset of glyph is c * 128 bytes, which can be computed by c << 7
	return ctx.term.FONT + ((unsigned int)c<<7);
}

/** Generic Font function. */
unsigned char* font_get_glyph_address_any(unsigned int c)
{
	return (unsigned char*) ( ctx.term.FONT + c * ctx.term.FONTCHARBYTES );
}

/** Compute some font variables from font size. */
void gfx_compute_font()
{
	ctx.term.FONTCHARBYTES = ctx.term.FONTWIDTH * ctx.term.FONTHEIGHT;
	ctx.term.FONTWIDTH_INTS = ctx.term.FONTWIDTH / 4 ;
	ctx.term.FONTWIDTH_REMAIN = ctx.term.FONTWIDTH % 4;
	ctx.cursor_buffer_size = ctx.term.FONTWIDTH * ctx.term.FONTHEIGHT;
	if (ctx.cursor_buffer)
	{
		free(ctx.cursor_buffer);
		ctx.cursor_buffer = 0;
		ctx.cursor_buffer_ready = 0;
	}
	ctx.cursor_buffer = (unsigned char*)malloc(ctx.cursor_buffer_size);
	local_memset(ctx.cursor_buffer, 0, ctx.cursor_buffer_size);

	// set logical terminal size
    ctx.term.WIDTH = ctx.W / ctx.term.FONTWIDTH;
    ctx.term.HEIGHT= ctx.H / ctx.term.FONTHEIGHT;
    gfx_term_save_cursor_content();
}

/** Sets the display variables. This is called by initialize_framebuffer when setting mode.
 * Default to 8x8 font if no other font was selected before.
 * @param p_framebuffer Framebuffer address as given by DMA
 * @param width Pixel width
 * @param height Pixel height
 * @param bpp Bit depth
 * @param pitch Line byte pitch as given by DMA
 * @param size Byte size for framebuffer
 */
void gfx_set_env( void* p_framebuffer, unsigned int width, unsigned int height, unsigned int bpp, unsigned int pitch, unsigned int size )
{
    dma_init();

    // set default font if needed
    if (ctx.term.FONT == 0) {
    	gfx_term_set_font(8,16);
    }

    // Store DMA framebuffer infos
    ctx.pfb = p_framebuffer;
    ctx.W = width;
    ctx.H = height;
    ctx.Pitch = pitch;
    ctx.size = size;
    ctx.bpp = bpp;

    // store terminal sizes and informations
    ctx.term.WIDTH = ctx.W / ctx.term.FONTWIDTH;
    ctx.term.HEIGHT= ctx.H / ctx.term.FONTHEIGHT;
    ctx.term.cursor_row = ctx.term.cursor_col = 0;
    ctx.term.cursor_visible = 1;
    ctx.term.state.next = state_fun_normaltext;

    // set default colors
    gfx_set_fg(15);
    gfx_set_bg(0);
    gfx_term_render_cursor();
}

/** Sets the background color. */
void gfx_set_bg( GFX_COL col )
{
    ctx.bg = col;
    // fill precomputed 4 bytes integer
    unsigned char* p = (unsigned char*)&ctx.bg32;
    for (size_t i = 0 ; i < sizeof ctx.fg32 ; i++)
    	*(p++) = col;
}

/** Sets the foreground color. */
void gfx_set_fg( GFX_COL col )
{
    ctx.fg = col;
    // fill precomputed 4 bytes integer
    unsigned char* p = (unsigned char*)&ctx.fg32;
    for (size_t i = 0 ; i < sizeof ctx.fg32 ; i++)
    	*(p++) = col;
}

/** Swaps the foreground and background colors. */
void gfx_swap_fg_bg()
{
    GFX_COL safe_fg = ctx.fg;
    gfx_set_fg(ctx.bg);
    gfx_set_bg(safe_fg);
}

/** Returns the character terminal size. */
void gfx_get_term_size( unsigned int* rows, unsigned int* cols )
{
    *rows = ctx.term.HEIGHT;
    *cols = ctx.term.WIDTH;
}

/** Returns the pixel display sizes . */
void gfx_get_gfx_size( unsigned int* width, unsigned int* height )
{
    *width = ctx.W;
    *height = ctx.H;
}

/** Sets the transparent color for transparent mode.
 *
 */
void gfx_set_transparent_color( GFX_COL color )
{
	ctx.transparentcolor = color;
}

/** Draw a sprite in normal mode.
 * The sprite pixels overwrite the existing background.
 * NB: Foreground and background color are not used by sprites.
 */
void gfx_put_sprite_NORMAL( unsigned char* p_sprite, unsigned int x, unsigned int y )
{
    unsigned char* pf = PFB(x,y);
    unsigned int *p_spr_32 = (unsigned int*)p_sprite;
    unsigned int width  = *p_spr_32; p_spr_32++;
    unsigned int height = *p_spr_32; p_spr_32++;

    unsigned int i,j;
    unsigned char* pspr = (unsigned char*)p_spr_32;

	for( i=0; i<height; ++i )
	{
		for( j=0; j<width; ++j )
		{
			*pf++ = *pspr++;
		}
		pf += ctx.Pitch - width;
	}
}
/** Draw a sprite in normal mode.
 * The sprite pixels are XORed with the existing background.
 * Interesting side effects:
 * - sprite pixel drawn over a black background (color 00) keep the original sprite color
 * - sprite color 0 pixel doesn't modify the existing background
 * NB: Foreground and background color are not used by sprites.
 */
void gfx_put_sprite_XOR( unsigned char* p_sprite, unsigned int x, unsigned int y )
{
    unsigned char* pf = PFB(x,y);
    unsigned int *p_spr_32 = (unsigned int*)p_sprite;
    unsigned int width  = *p_spr_32; p_spr_32++;
    unsigned int height = *p_spr_32; p_spr_32++;

    unsigned int i,j;
    unsigned char* pspr = (unsigned char*)p_spr_32;

	for( i=0; i<height; ++i )
	{
		for( j=0; j<width; ++j )
		{
			unsigned char bkg = *pf;
			unsigned char pix = *pspr++;
			*pf++ = pix ^ bkg ;
		}
		pf += ctx.Pitch - width;
	}
}
/** Draw a transparent sprite. Pixels with the transparent color are not drawn,
 * leaving the existing background visible. By default, 00 is the transparent color.
 */
void gfx_put_sprite_TRANSPARENT( unsigned char* p_sprite, unsigned int x, unsigned int y )
{
    unsigned char* pf = PFB(x,y);
    unsigned int *p_spr_32 = (unsigned int*)p_sprite;
    unsigned int width  = *p_spr_32; p_spr_32++;
    unsigned int height = *p_spr_32; p_spr_32++;

    unsigned int i,j;
    unsigned char* pspr = (unsigned char*)p_spr_32;

	for( i=0; i<height; ++i )
	{
		for( j=0; j<width; ++j )
		{
			unsigned char pix = *pspr++;
			if (pix != ctx.transparentcolor)
				*pf = pix;
			pf++;
		}
		pf += ctx.Pitch - width;
	}
}

/** Draw a sprite inthe current drawing mode.
 */
draw_putsprite_fun (*gfx_put_sprite) = gfx_put_sprite_NORMAL;

/** Sets the whole display to background color. */
void gfx_clear()
{
#if ENABLED(GFX_USE_DMA)
    unsigned int* BG = (unsigned int*)mem_2uncached( mem_buff_dma );
    // Fill the 16 bytes of dma buffer with background color
    *BG = ctx.bg32;
    *(BG+1) = *BG;
    *(BG+2) = *BG;
    *(BG+3) = *BG;

    dma_enqueue_operation( BG, 
            (unsigned int *)( ctx.pfb ), 
            ctx.size,
            0,
            DMA_TI_DEST_INC );

    dma_execute_queue();
    while( DMA_CHAN0_BUSY ); // Busy wait for DMA
#else
    unsigned char* pf = ctx.pfb;
    unsigned char* pfb_end = pf + ctx.size;
    while(pf < pfb_end)
        *pf++ = ctx.bg;
#endif
}

/** TODO: */
void gfx_scroll_down_dma( unsigned int npixels )
{
    unsigned int* BG = (unsigned int*)mem_2uncached( mem_buff_dma );
    const unsigned int bg = ctx.bg32;
    *BG = bg;
    *(BG+1) = bg;
    *(BG+2) = bg;
    *(BG+3) = bg;
    unsigned int line_height = ctx.Pitch * npixels;

    
    dma_enqueue_operation( (unsigned int *)( ctx.pfb + line_height ), 
                           (unsigned int *)( ctx.pfb ), 
                           (ctx.size - line_height),
                           0,
                           DMA_TI_SRC_INC | DMA_TI_DEST_INC );

    dma_enqueue_operation( BG, 
                           (unsigned int *)( ctx.pfb + ctx.size -line_height ), 
                           line_height,
                           0,
                           DMA_TI_DEST_INC );
}

/** TODO: */
void gfx_scroll_down( unsigned int npixels )
{
#if ENABLED(GFX_USE_DMA)
    gfx_scroll_down_dma( npixels );
    dma_execute_queue();
#else
    unsigned int* pf_src = (unsigned int*)( ctx.pfb + ctx.Pitch*npixels);
    unsigned int* pf_dst = (unsigned int*)ctx.pfb;
    const unsigned int* const pfb_end = (unsigned int*)( ctx.pfb + ctx.size );

    while( pf_src < pfb_end )
        *pf_dst++ = *pf_src++;

    // Fill with bg at the bottom
    const unsigned int BG = ctx.bg32;
    while( pf_dst < pfb_end )
        *pf_dst++ = BG;

#endif
}

/** TODO: comments */
void gfx_scroll_up( unsigned int npixels )
{
    unsigned int* pf_dst = (unsigned int*)( ctx.pfb + ctx.size ) -1;
    unsigned int* pf_src = (unsigned int*)( ctx.pfb + ctx.size - ctx.Pitch*npixels) -1;
    const unsigned int* const pfb_end = (unsigned int*)( ctx.pfb );

    while( pf_src >= pfb_end )
        *pf_dst-- = *pf_src--;
    
    // Fill with bg at the top
    const unsigned int BG = ctx.bg32;
    while( pf_dst >= pfb_end )
        *pf_dst-- = BG;
}

/** TODO: comments */
void gfx_fill_rect_dma( unsigned int x, unsigned int y, unsigned int width, unsigned int height )
{
    unsigned int* FG = (unsigned int*)mem_2uncached( mem_buff_dma )+4;
    const unsigned int fg = ctx.fg32;
    unsigned int* PFG = FG;
    for (size_t i = 0 ; i < sizeof ctx.fg32 ; i++)
    	*(PFG++) = fg;

    dma_enqueue_operation( FG, 
                           (unsigned int *)( PFB(x,y) ), 
                           (((height-1) & 0xFFFF )<<16) | (width & 0xFFFF ),
                           ((ctx.Pitch-width) & 0xFFFF)<<16, /* bits 31:16 destination stride, 15:0 source stride */
                           DMA_TI_DEST_INC | DMA_TI_2DMODE );
}

/** TODO: */
void gfx_fill_rect( unsigned int x, unsigned int y, unsigned int width, unsigned int height )
{
    if( x >= ctx.W || y >= ctx.H )
        return;

    if( x+width > ctx.W )
        width = ctx.W-x;

    if( y+height > ctx.H )
        height = ctx.H-y;

#if ENABLED(GFX_USE_DMA)
    gfx_fill_rect_dma( x, y, width, height );
    dma_execute_queue();
#else
    while( height-- )
    {
        unsigned char* pf = PFB(x,y);
        const unsigned char* const pfb_end = pf + width;

        while( pf < pfb_end )
            *pf++ = ctx.fg;

        ++y;
    }
#endif
}

/** TODO */
void gfx_draw_circle(unsigned int x0, unsigned int y0, unsigned int rad)
{
    register unsigned char* pfb;
    int f = 1 - rad;
    int ddF_x = 0;
    int ddF_y = -2 * rad;
    int x = 0;
    int y = rad;
    int xdraw,ydraw;

    //setPixel(x0, y0 + rad);
    ydraw = y0+rad;
    if (ydraw < (int)ctx.H)
    {
        pfb = PFB(x0,ydraw);
        *pfb = ctx.fg;
    }
    
    //setPixel(x0, y0 - rad);
    ydraw -= rad+rad;
    if (ydraw >= 0)
    {
        pfb = PFB(x0,ydraw);
        *pfb = ctx.fg;
    }
    
    //setPixel(x0 + rad, y0);
    xdraw = x0+rad;
    if (xdraw < (int)ctx.W)
    {
        pfb = PFB(xdraw,y0);
        *pfb = ctx.fg;
    }
    
    //setPixel(x0 - rad, y0);
    xdraw -= rad+rad;
    if (xdraw >= 0)
    {
        pfb = PFB(xdraw,y0);
        *pfb = ctx.fg;
    }
    

    while(x < y)
    {
        if(f >= 0)
        {
          y--;
          ddF_y += 2;
          f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x + 1;

        //setPixel(x0 + x, y0 + y);
        xdraw = x0+x;
        ydraw = y0+y;
        if ((xdraw < (int)ctx.W) && (ydraw < (int)ctx.H))
        {
            pfb = PFB(xdraw,ydraw);
            *pfb = ctx.fg;
        }
        
        //setPixel(x0 - x, y0 + y);
        xdraw = x0-x;
        if ((xdraw >= 0) && (ydraw < (int)ctx.H))
        {
            pfb = PFB(xdraw,ydraw);
            *pfb = ctx.fg;
        }
        
        //setPixel(x0 + x, y0 - y);
        xdraw = x0+x;
        ydraw = y0-y;
        if ((xdraw < (int)ctx.W) && (ydraw >= 0))
        {
            pfb = PFB(xdraw,ydraw);
            *pfb = ctx.fg;
        }
        
        //setPixel(x0 - x, y0 - y);
        xdraw = x0-x;
        if ((xdraw >= 0) && (ydraw >= 0))
        {
            pfb = PFB(xdraw,ydraw);
            *pfb = ctx.fg;
        }
        
        //setPixel(x0 + y, y0 + x);
        xdraw = x0+y;
        ydraw = y0+x;
        if ((xdraw < (int)ctx.W) && (ydraw < (int)ctx.H))
        {
            pfb = PFB(xdraw,ydraw);
            *pfb = ctx.fg;
        }
        
        //setPixel(x0 - y, y0 + x);
        xdraw = x0-y;
        if ((xdraw >= 0) && (ydraw < (int)ctx.H))
        {
            pfb = PFB(xdraw,ydraw);
            *pfb = ctx.fg;
        }
        
        //setPixel(x0 + y, y0 - x);
        xdraw = x0+y;
        ydraw = y0-x;
        if ((xdraw < (int)ctx.W) && (ydraw >= 0))
        {
            pfb = PFB(xdraw,ydraw);
            *pfb = ctx.fg;
        }
        
        //setPixel(x0 - y, y0 - x);
        xdraw = x0-y;
        if ((xdraw >= 0) && (ydraw >= 0))
        {
            pfb = PFB(xdraw,ydraw);
            *pfb = ctx.fg;
        }
    }
}

// Draw a horizontal line
void gfx_draw_hor_line(int x0, int y0, unsigned int width)
{
    if ((y0 < 0) || (y0 >= (int)ctx.H)) return;
    if (x0 >= (int)ctx.W) return;
    
    int diff;
    
    if (x0 < 0)
    {
        diff = 0-x0;
        x0 = 0;
        width-=diff;
    }
    if (x0+width >= ctx.W)
    {
        width-=ctx.W-x0;
    }
    
    unsigned int i;
    register unsigned char* pfb = PFB(x0,y0);
    for (i=0;i<width;i++)
    {
        *pfb++ = ctx.fg;
        //pfb++;
    }
}

// Draw a filled circle
void gfx_draw_filled_circle(unsigned int x0, unsigned int y0, unsigned int rad)
{
    int xoff = 0;
    int yoff = rad;
    int balance = -rad;
    int p0,p1,w0,w1;
    
    while (xoff <= yoff)
    {
        p0 = x0 - xoff;
        p1 = x0 - yoff;
        
        w0 = xoff + xoff;
        w1 = yoff + yoff;
        
        gfx_draw_hor_line(p0, y0 + yoff, w0);
        gfx_draw_hor_line(p0, y0 - yoff, w0);
        
        gfx_draw_hor_line(p1, y0 + xoff, w1);
        gfx_draw_hor_line(p1, y0 - xoff, w1);
        
        balance += xoff+xoff+1;
        xoff++;

        //if ((balance += xoff++ + xoff)>= 0)
        if (balance>= 0)
        {
            yoff--;
            balance -= yoff+yoff;
            //balance-=--yoff+yoff;
        }
    }
}

/** TODO: */
void gfx_clear_rect( unsigned int x, unsigned int y, unsigned int width, unsigned int height )
{
    GFX_COL curr_fg = ctx.fg;
    ctx.fg = ctx.bg;
    gfx_fill_rect(x,y,width,height);
    ctx.fg = curr_fg;
}

/** TODO: */
void gfx_line( int x0, int y0, int x1, int y1 )
{
    x0 = MAX( MIN(x0, (int)ctx.W), 0 );
    y0 = MAX( MIN(y0, (int)ctx.H), 0 );
    x1 = MAX( MIN(x1, (int)ctx.W), 0 );
    y1 = MAX( MIN(y1, (int)ctx.H), 0 );
    
    register unsigned char* pfb;
    int e2;
    int dx =  __abs__(x1-x0);
    int sx = x0<x1 ? 1 : -1;
    int dy = -__abs__(y1-y0);
    int sy = y0<y1 ? 1 : -1;
    int err = dx+dy;  /* error value e_xy */
    
    while (1)   /* loop */
    {
        // draw pixel
        pfb = PFB(x0,y0);
        *pfb = ctx.fg;

        if ((x0==x1) && (y0==y1)) break;
        e2 = 2*err;
        if (e2 >= dy)
        {
            err += dy; /* e_xy+e_x > 0 */
            x0 += sx;
        }
        if (e2 <= dx) /* e_xy+e_y < 0 */
        {
            err += dx;
            y0 += sy;
        }
    }
}

/** Display a character at a position. The character is drawn using
 * foreground color and pixels OFF are erased using the background color.
 *  NB: Characters with codes from 0 to 31 are displayed using current font and don't have any control effect.
 *	@param row the character line number (0 = top screen)
 *	@param col the character column (0 = left)
 *	@param c the character code from 0 to 255.
 */
void gfx_putc_NORMAL( unsigned int row, unsigned int col, unsigned char c )
{
    if( col >= ctx.term.WIDTH )
        return;
    if( row >= ctx.term.HEIGHT )
        return;

    const unsigned int pixcol = col * ctx.term.FONTWIDTH;
    const unsigned int pixrow = row * ctx.term.FONTHEIGHT;

    if (ctx.term.FONTWIDTH == 8)
    {
    	// optimized original code drawing 4 pixels at once using 32-bit ints
        const unsigned int FG = ctx.fg32;
        const unsigned int BG = ctx.bg32;
        const unsigned int int_stride = (ctx.Pitch>>2) - 2; // 2 ints = 8 pixels = 1 character
		register unsigned int* p_glyph = (unsigned int*)ctx.term.font_getglyph(c);
		register unsigned char h = ctx.term.FONTHEIGHT;
		register unsigned int* pf = (unsigned int*)PFB(pixcol, pixrow);
		while(h--)
		{
			// Loop unrolled for 8xH fonts
			register unsigned int gv = *p_glyph++;
			// Each byte is 00 for background pixel or FF for foreground pixel
			*pf++ =  (gv & FG) | ( ~gv & BG );
			gv = *p_glyph++;
			*pf++ =  (gv & FG) | ( ~gv & BG );
			pf += int_stride;
		}
    }
    else
    {
		// number of bytes to next line start position in framebuffer
    	const unsigned int byte_stride = ctx.Pitch - ctx.term.FONTWIDTH;
    	// address of character in font
		register unsigned char* p_glyph = (unsigned char*)ctx.term.font_getglyph(c);
		// copy from font to framebuffer for each font line
		register unsigned char h = ctx.term.FONTHEIGHT;
		register unsigned char* pf = (unsigned char*)PFB(pixcol, pixrow);
		while( h-- )
		{
			unsigned int w = ctx.term.FONTWIDTH;
			while( w-- )
			{
				register unsigned char gv = *p_glyph++;	// get 1 pixel of font glyph
				if (gv)
					*pf++ = ctx.fg;
				else
					*pf++ = ctx.bg;
			}
			pf += byte_stride;
		}
    }
}

/** Displays a character at a position, XORing it with current background.
 *  The pixels of the character are drawn by XORing the foreground color with existing background.
 *  The pixels OFF are ignored, leaving existing background untouched.
 *  NB: The background color is not used in this mode.
 *  NB: Characters with codes from 0 to 31 are displayed using current font and don't have any control effect.
 *	@param row the character line number (0 = top screen)
 *	@param col the character column (0 = left)
 *	@param c the character code from 0 to 255.
 */
void gfx_putc_XOR( unsigned int row, unsigned int col, unsigned char c )
{
    if( col >= ctx.term.WIDTH )
        return;
    if( row >= ctx.term.HEIGHT )
        return;

    const unsigned int pixcol = col * ctx.term.FONTWIDTH;
    const unsigned int pixrow = row * ctx.term.FONTHEIGHT;
	const unsigned int byte_stride = ctx.Pitch - ctx.term.FONTWIDTH;
	register unsigned char* p_glyph = (unsigned char*)ctx.term.font_getglyph(c);
	register unsigned char h = ctx.term.FONTHEIGHT;
	register unsigned char* pf = (unsigned char*)PFB(pixcol, pixrow);
	while( h-- )
	{
		unsigned int w = ctx.term.FONTWIDTH;
		while( w-- )
		{
			register unsigned char gv = *p_glyph++;
			if (gv)
				*pf = ctx.fg ^ gv;
			pf++;
		}
		pf += byte_stride;
	}
}

/** Displays a character at a position in transparent mode.
 *  The pixels of the character are drawn with the foreground color.
 *  The pixels OFF are ignored, leaving existing background untouched.
 *  NB: The background color is not used in this mode.
 *  NB: The transparent color is not used for drawing characters.
 *  NB: Characters with codes from 0 to 31 are displayed using current font and don't have any control effect.
 *	@param row the character line number (0 = top screen)
 *	@param col the character column (0 = left)
 *	@param c the character code from 0 to 255.
 */
void gfx_putc_TRANSPARENT( unsigned int row, unsigned int col, unsigned char c )
{
    if( col >= ctx.term.WIDTH )
        return;
    if( row >= ctx.term.HEIGHT )
        return;

    const unsigned int pixcol = col * ctx.term.FONTWIDTH;
    const unsigned int pixrow = row * ctx.term.FONTHEIGHT;
	const unsigned int byte_stride = ctx.Pitch - ctx.term.FONTWIDTH;
	register unsigned char* p_glyph = (unsigned char*)ctx.term.font_getglyph(c);
	register unsigned char h = ctx.term.FONTHEIGHT;
	register unsigned char* pf = (unsigned char*)PFB(pixcol, pixrow);
	while( h-- )
	{
		unsigned int w = ctx.term.FONTWIDTH;
		while( w-- )
		{
			register unsigned char gv = *p_glyph++;
			if (gv)
				*pf = ctx.fg;
			pf++;
		}
		pf += byte_stride;
	}
}

/** Displays a character in current drawing mode. Characters with codes from 0 to 31
 *  are displayed using current font and don't have any control effect.
 *	@param row the character line number (0 = top screen)
 *	@param col the character column (0 = left)
 *	@param c the character code from 0 to 255.
 */
draw_putc_fun (*gfx_putc) = &gfx_putc_NORMAL;

/** Sets the drawing mode for sprites and characters.
 * For Sprites, transparent mode uses the transparent color (00 by default).
 * @param mode the new drawing mode: drawingNORMAL, drawingXOR or drawingTRANSPARENT.
 */
void gfx_set_drawing_mode( DRAWING_MODE mode )
{
	ctx.mode = mode;
	switch (mode)
	{
	case drawingNORMAL:
		gfx_putc = gfx_putc_NORMAL;
		gfx_put_sprite = gfx_put_sprite_NORMAL;
		break;
	case drawingXOR:
		gfx_putc = gfx_putc_XOR;
		gfx_put_sprite = gfx_put_sprite_XOR;
		break;
	case drawingTRANSPARENT:
		gfx_putc = gfx_putc_TRANSPARENT;
		gfx_put_sprite = gfx_put_sprite_TRANSPARENT;
		break;
	}
}

/** Restore saved content under cursor.
 *
 */
void gfx_restore_cursor_content()
{
	if (!ctx.cursor_buffer_ready) return;

    unsigned char* pb = ctx.cursor_buffer;
    unsigned char* pfb = (unsigned char*)PFB( ctx.term.cursor_col * ctx.term.FONTWIDTH, ctx.term.cursor_row * ctx.term.FONTHEIGHT );
    const unsigned int byte_stride = ctx.Pitch - ctx.term.FONTWIDTH;
    unsigned int h = ctx.term.FONTHEIGHT;
    while(h--)
    {
    	unsigned int w = ctx.term.FONTWIDTH;
    	while (w--)
    	{
    		*pfb++ = *pb++;
    	}
        pfb += byte_stride;
    }

    //cout("cursor restored");cout_d(ctx.term.cursor_row);cout("-");cout_d(ctx.term.cursor_col);cout_endl();
}

/** Saves framebuffer content in the cursor bufffer so t can be restored later. */
void gfx_term_save_cursor_content()
{
	unsigned char* pb = ctx.cursor_buffer;
	unsigned char* pfb = (unsigned char*)PFB(
	ctx.term.cursor_col * ctx.term.FONTWIDTH,
	ctx.term.cursor_row * ctx.term.FONTHEIGHT );
	const unsigned int byte_stride = ctx.Pitch - ctx.term.FONTWIDTH;//$$ adjust if not 8 width?
	unsigned int h = ctx.term.FONTHEIGHT;
	while(h--)
    {
    	int w = ctx.term.FONTWIDTH;
    	while (w--)
    	{
    		*pb++ = *pfb++;
    	}
        pfb += byte_stride;
    }
	ctx.cursor_buffer_ready = 1;
}

/** Saves framebuffer content that is going to be replaced by the cursor and update
	the new content.
*/
void gfx_term_render_cursor()
{
    unsigned char* pb = ctx.cursor_buffer;
    unsigned char* pfb = (unsigned char*)PFB(
    		ctx.term.cursor_col * ctx.term.FONTWIDTH,
    		ctx.term.cursor_row * ctx.term.FONTHEIGHT );
    const unsigned int byte_stride = ctx.Pitch - ctx.term.FONTWIDTH;//$$ adjust if not 8 width?
    unsigned int h = ctx.term.FONTHEIGHT;

    if( ctx.term.cursor_visible )
        while(h--)
        {
        	int w = ctx.term.FONTWIDTH;
        	while (w--)
        	{
        		*pb++ = *pfb; *pfb = ~*pfb; pfb++;
        	}
            pfb += byte_stride;
        }
    else
        while(h--)
        {
        	int w = ctx.term.FONTWIDTH;
        	while (w--)
        	{
        		*pb++ = *pfb++;
        	}
            pfb += byte_stride;
        }
    ctx.cursor_buffer_ready = 1;
}

/** Fill cursor buffer with the current background and framebuffer with fg.
 */
void gfx_term_render_cursor_newline_dma()
{
    //
    const unsigned int BG = ctx.bg32; // 4 pixels
    unsigned int nwords = ctx.cursor_buffer_size / 4;
    unsigned int* pb = (unsigned int*)ctx.cursor_buffer;
    while( nwords-- )
    {
        *pb++ = BG;
    }

    if( ctx.term.cursor_visible )
        gfx_fill_rect_dma( ctx.term.cursor_col * ctx.term.FONTWIDTH, ctx.term.cursor_row * ctx.term.FONTHEIGHT, ctx.term.FONTWIDTH, ctx.term.FONTHEIGHT );
}

/** Draws a character string and handle control characters. */
void gfx_term_putstring( const char* str )
{
    while( *str )
    {
        while( DMA_CHAN0_BUSY ); // Busy wait for DMA

        int checkscroll = 1;
        switch( *str )
        {
            case '\r':
                gfx_restore_cursor_content();
                ctx.term.cursor_col = 0;
                if( ctx.term.cursor_row < ctx.term.HEIGHT ) gfx_term_render_cursor();
                break;

            case '\n':
                gfx_restore_cursor_content();
                ++ctx.term.cursor_row;
                ctx.term.cursor_col = 0;
                if( ctx.term.cursor_row < ctx.term.HEIGHT ) gfx_term_render_cursor();
                break;

            case 0x09: /* tab */
                gfx_restore_cursor_content();
                ctx.term.cursor_col += 1;
                ctx.term.cursor_col =  MIN( ctx.term.cursor_col + ctx.term.tab_pos - ctx.term.cursor_col%ctx.term.tab_pos, ctx.term.WIDTH-1 );
                gfx_term_render_cursor();
                break;

            case 0x08:
            case 0x7F:
                /* backspace */
                if( ctx.term.cursor_col>0 )
                {
                    gfx_restore_cursor_content();
                    --ctx.term.cursor_col;
                    gfx_clear_rect( ctx.term.cursor_col*ctx.term.FONTWIDTH, ctx.term.cursor_row*ctx.term.FONTHEIGHT, ctx.term.FONTWIDTH, ctx.term.FONTHEIGHT );
                    gfx_term_render_cursor();
                }
                break;

            case 0xC:
                /* new page */
                gfx_term_move_cursor(0,0);
                gfx_term_clear_screen();
                break;


            default:
            	checkscroll = ctx.term.state.next( *str, &(ctx.term.state) );
                break;
        }

        if( checkscroll && (ctx.term.cursor_col >= ctx.term.WIDTH ))
        {
            gfx_restore_cursor_content();
            ++ctx.term.cursor_row;
            ctx.term.cursor_col = 0;
            gfx_term_render_cursor();
        }

        if( checkscroll && (ctx.term.cursor_row >= ctx.term.HEIGHT ))
        {
            gfx_restore_cursor_content();
            --ctx.term.cursor_row;

            gfx_scroll_down_dma(ctx.term.FONTHEIGHT);
            gfx_term_render_cursor_newline_dma();
            dma_execute_queue();
        }

        ++str;
    }
}


void gfx_term_set_cursor_visibility( unsigned char visible )
{
    ctx.term.cursor_visible = visible;
}


void gfx_term_move_cursor( unsigned int row, unsigned int col )
{
    gfx_restore_cursor_content();
    ctx.term.cursor_row = MIN(ctx.term.HEIGHT-1, row );
    ctx.term.cursor_col = MIN(ctx.term.WIDTH-1, col );
    gfx_term_render_cursor();
}


void gfx_term_move_cursor_d( int delta_row, int delta_col )
{
    if( (int)ctx.term.cursor_col+delta_col < 0 )
        delta_col = 0;

    if( (int)ctx.term.cursor_row+delta_row < 0 )
        delta_row = 0;
    gfx_term_move_cursor( ctx.term.cursor_row+delta_row, ctx.term.cursor_col+delta_col );
}


void gfx_term_save_cursor()
{
    ctx.term.saved_cursor[0] = ctx.term.cursor_row;
    ctx.term.saved_cursor[1] = ctx.term.cursor_col;
}


void gfx_term_restore_cursor()
{
    gfx_restore_cursor_content();
    ctx.term.cursor_row = ctx.term.saved_cursor[0];
    ctx.term.cursor_col = ctx.term.saved_cursor[1];
    gfx_term_render_cursor();
}


void gfx_term_clear_till_end()
{
    gfx_swap_fg_bg();
    gfx_fill_rect( (ctx.term.cursor_col+1) * ctx.term.FONTWIDTH, ctx.term.cursor_row * ctx.term.FONTWIDTH, ctx.W, ctx.term.FONTHEIGHT );
    gfx_swap_fg_bg();
}


void gfx_term_clear_till_cursor()
{
    gfx_swap_fg_bg();
    gfx_fill_rect( 0, ctx.term.cursor_row * ctx.term.FONTHEIGHT, ctx.term.cursor_col * ctx.term.FONTWIDTH, ctx.term.FONTHEIGHT );
    gfx_swap_fg_bg();
}


void gfx_term_clear_line()
{
    gfx_swap_fg_bg();
    gfx_fill_rect( 0, ctx.term.cursor_row*ctx.term.FONTHEIGHT, ctx.W, ctx.term.FONTHEIGHT );
    gfx_swap_fg_bg();
    gfx_term_render_cursor();
}


void gfx_term_clear_screen()
{
    gfx_clear();
    gfx_term_render_cursor();
}

/** Set the font. */
void gfx_term_set_font(int width, int height)
{
	if (width == 8)
	{
		switch (height)
		{
		case 8:
			ctx.term.FONT = &G_FONT8X8_GLYPHS;
			ctx.term.FONTWIDTH = 8;
			ctx.term.FONTHEIGHT = 8;
			ctx.term.font_getglyph = font_get_glyph_address_8x8;
			gfx_compute_font();
			break;
		case 16:
			ctx.term.FONT = &G_FONT8X16_GLYPHS;
			ctx.term.FONTWIDTH = 8;
			ctx.term.FONTHEIGHT = 16;
			ctx.term.font_getglyph = font_get_glyph_address_8x16;
			gfx_compute_font();
			break;
		case 24:
			ctx.term.FONT = &G_FONT8X24_GLYPHS;
			ctx.term.FONTWIDTH = 8;
			ctx.term.FONTHEIGHT = 24;
			ctx.term.font_getglyph = font_get_glyph_address_8x24;
			gfx_compute_font();
			break;
		}
	}
}

/** Sets the tabulation width. */
void gfx_term_set_tabulation(int width)
{
	if (width < 0) width = 8;
	if (width > (int)ctx.term.WIDTH) width = (int)ctx.term.WIDTH;
	ctx.term.tab_pos = (unsigned int)width;
}

/**  Term ANSI prefix code */
#define TERM_ESCAPE_CHAR (0x1B)

/** State parsing functions */

/** Parse the last letter in ANSI sequence.
 *  Normal ANSI escape sequences assume previous parameters are stored as numbers in state->cmd_params[].
 *
 *  state->private_mode_char can hold a character in which case the process is not
 *  following ANSI or VT100 specifications:
 *
 *      ESC[# implements graphic commands and tests (prioritary)
 *      ESC[= implements settings change: mode (PC ANSI.SYS), font, tab width
 *      ESC[? implements some ANSI commands (save/restore cursor content)
 *
 *  Any other character will end the sequence.
 *
 *  @param ch the character to scan
 *	@param state points to the current state structure
 *	@return 1 if the terminal should handle line break and screen scroll returning from this call.
 *
 */
int state_fun_final_letter( char ch, scn_state *state )
{
	int retval = 1;// handle line break and screen scroll by default
    if( state->private_mode_char == '#' )
    {
        // Non-standard graphic commands and additionam features
        switch( ch )
        {
            case 'l':
                /* render line */
                if( state->cmd_params_size == 4 )
                {
                    gfx_line( state->cmd_params[0], state->cmd_params[1], state->cmd_params[2], state->cmd_params[3] );
                }
                retval = 0; // no terminal line break/scroll
            goto back_to_normal;
            break;
            case 'r':
                /* render a filled rectangle */
                if( state->cmd_params_size == 4 )
                {
                    gfx_fill_rect( state->cmd_params[0], state->cmd_params[1], state->cmd_params[2], state->cmd_params[3] );
                }
                retval = 0;
            goto back_to_normal;
            break;
            case 'R':
                /* render a rectangle */
                if( state->cmd_params_size == 4 )
                {
                    // x0;y0;width;height;
                    gfx_line( state->cmd_params[0], state->cmd_params[1], state->cmd_params[0]+state->cmd_params[2], state->cmd_params[1] ); // x0/y0 to x1/y0
                    gfx_line( state->cmd_params[0], state->cmd_params[1], state->cmd_params[0], state->cmd_params[1]+state->cmd_params[3] ); // x0/y0 to x0/y1
                    gfx_line( state->cmd_params[0]+state->cmd_params[2], state->cmd_params[1], state->cmd_params[0]+state->cmd_params[2], state->cmd_params[1]+state->cmd_params[3] ); // x1/y0 to x1/y1
                    gfx_line( state->cmd_params[0], state->cmd_params[1]+state->cmd_params[3], state->cmd_params[0]+state->cmd_params[2], state->cmd_params[1]+state->cmd_params[3] ); // x0/y1 to x1/y1
                }
                retval = 0; // no terminal line break/scroll
            goto back_to_normal;
            break;
            case 'C':
                /* render a circle */
                if (state->cmd_params_size == 3)
                {
                    gfx_draw_circle(state->cmd_params[0], state->cmd_params[1], state->cmd_params[2]);  // x, y, radius
                }
                retval = 0;
            goto back_to_normal;
            break;
            case 'c':
                /* render a filled circle */
                if (state->cmd_params_size == 3)
                {
                    gfx_draw_filled_circle(state->cmd_params[0], state->cmd_params[1], state->cmd_params[2]);  // x, y, radius
                }
                retval = 0;
            goto back_to_normal;
            case 'T':
                /* render a triangle */
                if (state->cmd_params_size == 6)
                {
                    gfx_line( state->cmd_params[0], state->cmd_params[1], state->cmd_params[2], state->cmd_params[3] );
                    gfx_line( state->cmd_params[2], state->cmd_params[3], state->cmd_params[4], state->cmd_params[5] );
                    gfx_line( state->cmd_params[4], state->cmd_params[5], state->cmd_params[0], state->cmd_params[1] );
                }
                retval = 0;
            goto back_to_normal;
            break;

            /** The following is only for debug purposes
            case 't':
            	// video test from pigfx.c
            	video_test(1000);
            	goto back_to_normal;
            	break;
            case 'T':
            	// video test from pigfx.c
            	video_line_test(1000 );
            	goto back_to_normal;
            	break;
            	*/
        }
    }

    if ( state->private_mode_char == '=' )
    {
    	// ANSI.SYS style mode changing
    	switch( ch )
    	{
    	case 'h': // set resolution mode on last parameter, ignore previous
    		if( state->cmd_params_size >= 1)
    		{
    			// parameter is the mode index in global array
    			if (state->cmd_params[state->cmd_params_size - 1] <= LAST_MODE_NUMBER)
    			{
        			struct DISPLAY_MODE_DEFINITION* p = & ALL_MODES[state->cmd_params[state->cmd_params_size-1]];
        			if (p->width > 0)
        			{
        				initialize_framebuffer( p->width, p->height, p->bpp);
        			}
    			}
    		}
    		goto back_to_normal;
    		break;

    	case 'f': // ESC=0f choose 8x8 font, ESC=1f for 8x16, ESC=2f for 8x24
    		if( state->cmd_params_size >= 1)
    		{
    			// parameter is the font number
    			switch (state->cmd_params[state->cmd_params_size - 1])
    			{
    			case 0:
    				gfx_term_set_font(8,8);
    				break;
				case 1:
					gfx_term_set_font(8,16);
					break;
				case 2:
					gfx_term_set_font(8,24);
					break;
				default:
					// ignore
					break;
    			}
    		}
    		goto back_to_normal;
    		break;

    	case 'm': // ESC=0m normal mode, ESC=1m XOR, ESC=2m TRANSPARENT
       		if( state->cmd_params_size >= 1)
        	{
				// parameter is the font number
				switch (state->cmd_params[state->cmd_params_size - 1])
				{
				case 1:
					gfx_set_drawing_mode(drawingXOR);
					break;
				case 2:
					gfx_set_drawing_mode(drawingTRANSPARENT);;
					break;
				default:
					gfx_set_drawing_mode(drawingNORMAL);
					break;
        		}
    		}
    		goto back_to_normal;
    		break;

    	case 't': // ESC=xxxt sets tabulation width
        	if( state->cmd_params_size >= 1)
        	{
        		gfx_term_set_tabulation(state->cmd_params[state->cmd_params_size - 1]);
        	}
        	goto back_to_normal;
        	break;

    	} // switch last letter after '=' and parameters
    } // private mode = '='

    // General 'ESC[' ANSI/VT100 commands
    switch( ch )
    {
        case 'l':
            if( state->private_mode_char == '?' && 
                state->cmd_params_size == 1 &&
                state->cmd_params[0] == 25 )
            {
                gfx_term_set_cursor_visibility(0);
                gfx_restore_cursor_content();
            }
            goto back_to_normal;
            break;

        case 'h':
            if( state->private_mode_char == '?' && 
                state->cmd_params_size == 1 &&
                state->cmd_params[0] == 25 )
            {
                gfx_term_set_cursor_visibility(1);
                gfx_term_render_cursor();
            }
            goto back_to_normal;
            break;

        case 'K':
            if( state->cmd_params_size== 1 )
            {
                switch(state->cmd_params[0] )
                {
                    case 0:
                        gfx_term_clear_till_end();
                        goto back_to_normal;
                    case 1:
                        gfx_term_clear_till_cursor();
                        goto back_to_normal;
                    case 2:
                        gfx_term_clear_line();
                        goto back_to_normal;
                    default:
                        goto back_to_normal;
                }
            }
            goto back_to_normal;
            break;
        
        case 'J':
            if( state->cmd_params_size==1 && state->cmd_params[0] ==2 )
            {
                gfx_term_move_cursor(0,0);
                gfx_term_clear_screen();
            }
            goto back_to_normal;
            break;

        case 'A':
            //if( state->cmd_params_size == 1 )
                gfx_term_move_cursor_d( -state->cmd_params[0], 0 );

            goto back_to_normal;
            break;

        case 'B':
            //if( state->cmd_params_size == 1 )
                gfx_term_move_cursor_d( state->cmd_params[0], 0 );

            goto back_to_normal;
            break;

        case 'C':
            //if( state->cmd_params_size == 1 )
                gfx_term_move_cursor_d( 0, state->cmd_params[0] );

            goto back_to_normal;
            break;

        case 'D':
            //if( state->cmd_params_size == 1 )
                gfx_term_move_cursor_d( 0, -state->cmd_params[0] );

            goto back_to_normal;
            break;

        case 'm':
            if( state->cmd_params_size == 1 && state->cmd_params[0]==0 )
            {
                gfx_set_bg(0);
                gfx_set_fg(15);
                goto back_to_normal;
            }
            if( state->cmd_params_size == 3 &&
                state->cmd_params[0]==38    &&
                state->cmd_params[1]==5 )
            {
                gfx_set_fg( state->cmd_params[2] );
                goto back_to_normal;
            }
            if( state->cmd_params_size == 3 &&
                state->cmd_params[0]==48    &&
                state->cmd_params[1]==5 )
            {
                gfx_set_bg( state->cmd_params[2] );
                goto back_to_normal;
            }
            goto back_to_normal;
            break;

        case 'f':
        case 'H':
            if( state->cmd_params_size == 2 )
            {
                int row = (state->cmd_params[0] - 1) % ctx.term.HEIGHT;
                int col = (state->cmd_params[1] - 1) % ctx.term.WIDTH; // 80 -> (79 % 80) -> 79, 81 -> (80 % 80) -> 0
                gfx_term_move_cursor(row, col);
            }
            else
                gfx_term_move_cursor(0,0);
            goto back_to_normal;
            break;

        case 's':
            gfx_term_save_cursor();
            goto back_to_normal;
            break;

        case 'u':
            gfx_term_restore_cursor();
            goto back_to_normal;
            break;

        default:
            goto back_to_normal;
    }

back_to_normal:
    // go back to normal text
    state->cmd_params_size = 0;
    state->next = state_fun_normaltext;
    return retval;
}

/** Read next digit of a parameter or a separator. */
int state_fun_read_digit( char ch, scn_state *state )
{
    if( ch>='0' && ch <= '9' )
    {
    	// make sure we have a parameter
    	if (state->cmd_params_size == 0)
    	{
    		state->cmd_params_size = 1;
    		state->cmd_params[0] = 0;
    	}
        // parse digit
        state->cmd_params[ state->cmd_params_size - 1] = state->cmd_params[ state->cmd_params_size - 1]*10 + (ch-'0');
        state->next = state_fun_read_digit; // stay on this state
        return 1;
    }
    if (ch == '.')
    {
        state->next = state_fun_ignore_digit;
         return 1;
    }
    if( ch == ';' )
    {
        // Another param will follow
        state->cmd_params_size++;
        state->cmd_params[ state->cmd_params_size-1 ] = 0;
        state->next = state_fun_read_digit; // stay on this state
        return 1;
    }

    // not a digit, call the final state
    state_fun_final_letter( ch, state );
    return 1;
}

/** Ignore next digits of a parameter until separator. */
int state_fun_ignore_digit( char ch, scn_state *state )
{
	if( ch>='0' && ch <= '9' )
	{
		return 1;
	}
	if( ch == ';' )
    {
        // Another param will follow
        state->cmd_params_size++;
        state->cmd_params[ state->cmd_params_size-1 ] = 0;
        state->next = state_fun_read_digit; // stay on this state
        return 1;
    }
    // not a digit, end
    state_fun_final_letter( ch, state );
    return 1;
}

/** Right after ESC, start a parameter when reading a digit or select the private mode character. */
int state_fun_selectescape( char ch, scn_state *state )
{
    if( ch>='0' && ch<='9' )
    {
        // start of a digit
        state->cmd_params_size = 1;
        state->cmd_params[ 0 ] = ch-'0';
        state->next = state_fun_read_digit;
        return 1;
    } 
    else
    {
        if( ch=='?' || ch=='#' || ch=='=' )
        {
            state->private_mode_char = ch;
            // A digit may follow
            state->cmd_params_size = 0;
            state->next = state_fun_read_digit;
            return 1;
        }
    } 

    // Already at the final letter
    state_fun_final_letter( ch, state );
    return 1;
}

/** Right after ESC, wait for a [.
 *  The special case ESC ESC is displaying ESC character and end the sequence.
 */
int state_fun_waitsquarebracket( char ch, scn_state *state )
{
    if( ch=='[' )
    {
        state->cmd_params[0]=1;
        state->private_mode_char=0; // reset private mode char
        state->next = state_fun_selectescape;
        return 1;
    }

    if( ch==TERM_ESCAPE_CHAR ) // Double ESCAPE prints the ESC character
    {
        gfx_putc( ctx.term.cursor_row, ctx.term.cursor_col, ch );
        ++ctx.term.cursor_col;
        gfx_term_render_cursor();
    }

    state->next = state_fun_normaltext;
    return 1;
}

/** Starting state when receiving a character to display.
 *  If the character is ESC, a sequence parsing state is entered.
 *  If none of the previous happened, the character is displayed using current font.
 */
int state_fun_normaltext( char ch, scn_state *state )
{
    if( ch==TERM_ESCAPE_CHAR )
    {
        state->next = state_fun_waitsquarebracket;
        return 1;
    }

    gfx_putc( ctx.term.cursor_row, ctx.term.cursor_col, ch );
    ++ctx.term.cursor_col;
    gfx_term_render_cursor();
    return 1;
}
