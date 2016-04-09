#ifndef _GFX_H_
#define _GFX_H_


typedef unsigned char GFX_COL;

extern void gfx_set_env( void* p_framebuffer, unsigned int width, unsigned int height, unsigned int pitch, unsigned int size ); 
extern void gfx_set_bg( GFX_COL col );
extern void gfx_set_fg( GFX_COL col );
extern void gfx_swap_fg_bg();
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
 * Renders a line from x0-y0 to x1-y1 
 */
extern void gfx_line( int x0, int y0, int x1, int y1 );

/*! 
 * Fills a rectangle with the background color 
 */
extern void gfx_clear_rect( unsigned int x, unsigned int y, unsigned int width, unsigned int height );

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



/*!
 *  Terminal
 *
 */
extern void gfx_term_putstring( unsigned char* str );
extern void gfx_term_set_cursor_visibility( unsigned char visible );
extern void gfx_term_move_cursor( unsigned int row, unsigned int col );
extern void gfx_term_move_cursor_d( int delta_row, int delta_col );
extern void gfx_term_save_cursor();
extern void gfx_term_restore_cursor();
extern void gfx_term_clear_till_end();
extern void gfx_term_clear_till_cursor();
extern void gfx_term_clear_line();
extern void gfx_term_clear_screen();


#endif
