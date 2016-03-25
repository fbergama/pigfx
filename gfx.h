#ifndef _GFX_H_
#define _GFX_H_


typedef unsigned char GFX_COL;

extern void gfx_set_env( void* p_framebuffer, unsigned int width, unsigned int height, unsigned int pitch, unsigned int size ); 
extern void gfx_set_bg( GFX_COL col );
extern void gfx_set_fg( GFX_COL col );
extern void gfx_get_term_size( unsigned int* rows, unsigned int* cols );

/*! 
 * Fills the entire framebuffer with the background color 
 */
extern void gfx_clear();


/*! 
 * Fills a rectangle with the foreground color 
 */
extern void gfx_fill_rect( unsigned int x, unsigned int y, unsigned int width, unsigned int height );


/*! 
 * Renders the character "c" at location (x,y)
 */
extern void gfx_putc( unsigned int row, unsigned int col, unsigned char c );


/*! 
 * Scrolls the entire framebuffer down (adding background color at the bottom)
 */
extern void gfx_scroll_down( unsigned int npixels );


/*! 
 * Scrolls the entire framebuffer up (adding background color at the top)
 */
extern void gfx_scroll_up( unsigned int npixels );

#endif
