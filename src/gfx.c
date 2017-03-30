#include "pigfx_config.h"
#include "gfx.h"
#include "console.h"
#include "dma.h"
#include "utils.h"

extern unsigned char G_FONT_GLYPHS;
static unsigned char* FNT = &G_FONT_GLYPHS;


#define MIN( v1, v2 ) ( ((v1) < (v2)) ? (v1) : (v2))
#define MAX( v1, v2 ) ( ((v1) > (v2)) ? (v1) : (v2))
#define PFB( X, Y ) ( ctx.pfb + Y*ctx.Pitch + X )

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

typedef struct SCN_STATE
{
    //state_fun* next;
    void (*next)( char ch, struct SCN_STATE *state );
    void (*after_read_digit)( char ch, struct SCN_STATE *state );

    unsigned int cmd_params[10];
    unsigned int cmd_params_size;
    char private_mode_char;
} scn_state;

/* state forward declarations */
typedef void state_fun( char ch, scn_state *state );
void state_fun_normaltext( char ch, scn_state *state );
void state_fun_read_digit( char ch, scn_state *state );



typedef struct {
    unsigned int W;
    unsigned int H;
    unsigned int Pitch;
    unsigned int size;
    unsigned char* pfb;

    struct
    {
        unsigned int WIDTH;
        unsigned int HEIGHT;
        unsigned int cursor_row;
        unsigned int cursor_col;
        unsigned int saved_cursor[2];
        char cursor_visible;

        scn_state state;
    } term;

    GFX_COL bg;
    GFX_COL fg;

    unsigned int cursor_buffer[16];

} FRAMEBUFFER_CTX;


static FRAMEBUFFER_CTX ctx;
unsigned int __attribute__((aligned(0x100))) mem_buff_dma[16];


void gfx_term_render_cursor();



void gfx_set_env( void* p_framebuffer, unsigned int width, unsigned int height, unsigned int pitch, unsigned int size )
{
    dma_init();

    ctx.pfb = p_framebuffer;
    ctx.W = width;
    ctx.H = height;
    ctx.Pitch = pitch;
    ctx.size = size;

    ctx.term.WIDTH = ctx.W / 8;
    ctx.term.HEIGHT= ctx.H / 8;
    ctx.term.cursor_row = ctx.term.cursor_col = 0;
    ctx.term.cursor_visible = 1;
    ctx.term.state.next = state_fun_normaltext;

    ctx.bg = 0;
    ctx.fg = 15;
    gfx_term_render_cursor();
}


void gfx_set_bg( GFX_COL col )
{
    ctx.bg = col;
}


void gfx_set_fg( GFX_COL col )
{
    ctx.fg = col;
}


void gfx_swap_fg_bg()
{
    GFX_COL aux = ctx.fg;
    ctx.fg = ctx.bg;
    ctx.bg = aux;
}

void gfx_get_term_size( unsigned int* rows, unsigned int* cols )
{
    *rows = ctx.term.HEIGHT;
    *cols = ctx.term.WIDTH;
}


void gfx_clear()
{
#if ENABLED(GFX_USE_DMA)
    unsigned int* BG = (unsigned int*)mem_2uncached( mem_buff_dma );
    *BG = ctx.bg<<24 | ctx.bg<<16 | ctx.bg<<8 | ctx.bg;
    *(BG+1) = *BG;
    *(BG+2) = *BG;
    *(BG+3) = *BG;

    dma_enqueue_operation( BG,
            (unsigned int *)( ctx.pfb ),
            ctx.size,
            0,
            DMA_TI_DEST_INC );

    dma_execute_queue();
#else
    unsigned char* pf = ctx.pfb;
    unsigned char* pfb_end = pf + ctx.size;
    while(pf < pfb_end)
        *pf++ = ctx.bg;
#endif
}


void gfx_scroll_down_dma( unsigned int npixels )
{
    unsigned int* BG = (unsigned int*)mem_2uncached( mem_buff_dma );
    *BG = ctx.bg<<24 | ctx.bg<<16 | ctx.bg<<8 | ctx.bg;
    *(BG+1) = *BG;
    *(BG+2) = *BG;
    *(BG+3) = *BG;
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
    const unsigned int BG = ctx.bg<<24 | ctx.bg<<16 | ctx.bg<<8 | ctx.bg;
    while( pf_dst < pfb_end )
        *pf_dst++ = BG;

#endif
}


void gfx_scroll_up( unsigned int npixels )
{
    unsigned int* pf_dst = (unsigned int*)( ctx.pfb + ctx.size ) -1;
    unsigned int* pf_src = (unsigned int*)( ctx.pfb + ctx.size - ctx.Pitch*npixels) -1;
    const unsigned int* const pfb_end = (unsigned int*)( ctx.pfb );

    while( pf_src >= pfb_end )
        *pf_dst-- = *pf_src--;

    // Fill with bg at the top
    const unsigned int BG = ctx.bg<<24 | ctx.bg<<16 | ctx.bg<<8 | ctx.bg;
    while( pf_dst >= pfb_end )
        *pf_dst-- = BG;
}


void gfx_fill_rect_dma( unsigned int x, unsigned int y, unsigned int width, unsigned int height )
{
    unsigned int* FG = (unsigned int*)mem_2uncached( mem_buff_dma )+4;
    *FG = ctx.fg<<24 | ctx.fg<<16 | ctx.fg<<8 | ctx.fg;
    *(FG+1) = *FG;
    *(FG+2) = *FG;
    *(FG+3) = *FG;

    dma_enqueue_operation( FG,
                           (unsigned int *)( PFB(x,y) ),
                           (((height-1) & 0xFFFF )<<16) | (width & 0xFFFF ),
                           ((ctx.Pitch-width) & 0xFFFF)<<16, /* bits 31:16 destination stride, 15:0 source stride */
                           DMA_TI_DEST_INC | DMA_TI_2DMODE );
}


void gfx_fill_rect( unsigned int x, unsigned int y, unsigned int width, unsigned int height )
{
#if ENABLED(GFX_USE_DMA)
    gfx_fill_rect_dma( x, y, width, height );
    dma_execute_queue();
#else
    if( x >= ctx.W || y >= ctx.H )
        return;

    if( x+width > ctx.W )
        width = ctx.W-x;

    if( y+height > ctx.H )
        height = ctx.H-y;


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


void gfx_clear_rect( unsigned int x, unsigned int y, unsigned int width, unsigned int height )
{
    GFX_COL curr_fg = ctx.fg;
    ctx.fg = ctx.bg;
    gfx_fill_rect(x,y,width,height);
    ctx.fg = curr_fg;
}


void gfx_line( int x0, int y0, int x1, int y1 )
{
    x0 = MAX( MIN(x0, (int)ctx.W), 0 );
    y0 = MAX( MIN(y0, (int)ctx.H), 0 );
    x1 = MAX( MIN(x1, (int)ctx.W), 0 );
    y1 = MAX( MIN(y1, (int)ctx.H), 0 );

    unsigned char qrdt = __abs__(y1 - y0) > __abs__(x1 - x0);

    if( qrdt ) {
        __swap__(&x0, &y0);
        __swap__(&x1, &y1);
    }
    if( x0 > x1 ) {
        __swap__(&x0, &x1);
        __swap__(&y0, &y1);
    }

    const int deltax = x1 - x0;
    const int deltay = __abs__(y1 - y0);
    register int error = deltax >> 1;
    register unsigned char* pfb;
    unsigned int nr = x1-x0;

    if( qrdt )
    {
        const int ystep = y0<y1 ? 1 : -1;
        pfb = PFB(y0,x0);
        while(nr--)
        {
            *pfb = ctx.fg;
            error = error - deltay;
            if( error < 0 )
            {
                pfb += ystep;
                error += deltax;
            }
            pfb += ctx.Pitch;
        }
    }
    else
    {
        const int ystep = y0<y1 ? ctx.Pitch : -ctx.Pitch;
        pfb = PFB(x0,y0);
        while(nr--)
        {
            *pfb = ctx.fg;
            error = error - deltay;
            if( error < 0 )
            {
                pfb += ystep;
                error += deltax;
            }
            pfb++;
        }
    }
}


void gfx_putc( unsigned int row, unsigned int col, unsigned char c )
{
    if( col >= ctx.term.WIDTH )
        return;

    if( row >= ctx.term.HEIGHT )
        return;

    const unsigned int FG = ctx.fg<<24 | ctx.fg<<16 | ctx.fg<<8 | ctx.fg;
    const unsigned int BG = ctx.bg<<24 | ctx.bg<<16 | ctx.bg<<8 | ctx.bg;
    const unsigned int stride = (ctx.Pitch>>2) - 2;

    register unsigned int* p_glyph = (unsigned int*)( FNT + ((unsigned int)c<<6) );
    register unsigned int* pf = (unsigned int*)PFB((col<<3), (row<<3));
    register unsigned char h=8;

    while(h--)
    {
        //unsigned int w=2;
        //while( w-- ) // Loop unrolled for 8x8 fonts
        {
            register unsigned int gv = *p_glyph++;
            *pf++ =  (gv & FG) | ( ~gv & BG );
            gv = *p_glyph++;
            *pf++ =  (gv & FG) | ( ~gv & BG );
        }
        pf += stride;
    }
}



/*
 * Terminal functions
 *
 */

void gfx_restore_cursor_content()
{
    // Restore framebuffer content that was overwritten by the cursor
    unsigned int* pb = (unsigned int*)ctx.cursor_buffer;
    unsigned int* pfb = (unsigned int*)PFB( ctx.term.cursor_col*8, ctx.term.cursor_row*8 );
    const unsigned int stride = (ctx.Pitch>>2) - 2;
    unsigned int h=8;
    while(h--)
    {
        *pfb++ = *pb++;
        *pfb++ = *pb++;
        pfb+=stride;
    }
    //cout("cursor restored");cout_d(ctx.term.cursor_row);cout("-");cout_d(ctx.term.cursor_col);cout_endl();
}


void gfx_term_render_cursor()
{
    // Save framebuffer content that is going to be replaced by the cursor and update
    // the new content
    //
    unsigned int* pb = (unsigned int*)ctx.cursor_buffer;
    unsigned int* pfb = (unsigned int*)PFB( ctx.term.cursor_col*8, ctx.term.cursor_row*8 );
    const unsigned int stride = (ctx.Pitch>>2) - 2;
    unsigned int h=8;

    if( ctx.term.cursor_visible )
        while(h--)
        {
            *pb++ = *pfb; *pfb = ~*pfb; pfb++;
            *pb++ = *pfb; *pfb = ~*pfb; pfb++;
            pfb+=stride;
        }
    else
        while(h--)
        {
            *pb++ = *pfb++;
            *pb++ = *pfb++;
            pfb+=stride;
        }
}


void gfx_term_render_cursor_newline_dma()
{
    // Fill cursor buffer with the current background and framebuffer with fg
    unsigned int BG = ctx.bg<<24 | ctx.bg<<16 | ctx.bg<<8 | ctx.bg;

    unsigned int nwords = 16;
    unsigned int* pb = (unsigned int*)ctx.cursor_buffer;
    while( nwords-- )
    {
        *pb++ = BG;
    }

    if( ctx.term.cursor_visible )
        gfx_fill_rect_dma( ctx.term.cursor_col*8, ctx.term.cursor_row*8, 8, 8 );
}


void gfx_term_putstring( const char* str )
{
    while( *str )
    {
        while( DMA_CHAN0_BUSY ); // Busy wait for DMA

        switch( *str )
        {
            case '\r':
                gfx_restore_cursor_content();
                ctx.term.cursor_col = 0;
                gfx_term_render_cursor();
                break;

            case '\n':
                gfx_restore_cursor_content();
                ++ctx.term.cursor_row;
                ctx.term.cursor_col = 0;
                gfx_term_render_cursor();
                break;

            case 0x09: /* tab */
                gfx_restore_cursor_content();
                ctx.term.cursor_col += 1;
                ctx.term.cursor_col =  MIN( ctx.term.cursor_col + 8 - ctx.term.cursor_col%8, ctx.term.WIDTH-1 );
                gfx_term_render_cursor();
                break;

            case 0x08:
            case 0x7F:
                /* backspace */
                if( ctx.term.cursor_col>0 )
                {
                    gfx_restore_cursor_content();
                    --ctx.term.cursor_col;
                    gfx_clear_rect( ctx.term.cursor_col*8, ctx.term.cursor_row*8, 8, 8 );
                    gfx_term_render_cursor();
                }
                break;

            case 0xC:
                /* new page */
                gfx_term_move_cursor(0,0);
                gfx_term_clear_screen();
                break;


            default:
                ctx.term.state.next( *str, &(ctx.term.state) );
                break;
        }

        if( ctx.term.cursor_col >= ctx.term.WIDTH )
        {
            gfx_restore_cursor_content();
            ++ctx.term.cursor_row;
            ctx.term.cursor_col = 0;
            gfx_term_render_cursor();
        }

        if( ctx.term.cursor_row >= ctx.term.HEIGHT )
        {
            gfx_restore_cursor_content();
            --ctx.term.cursor_row;

            gfx_scroll_down_dma(8);
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
    ctx.term.cursor_row = MIN( ctx.term.HEIGHT-1, row );
    ctx.term.cursor_col = MIN( ctx.term.WIDTH-1, col );

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
    gfx_fill_rect( (ctx.term.cursor_col+1) * 8, ctx.term.cursor_row*8, ctx.W, 8 );
    gfx_swap_fg_bg();
}


void gfx_term_clear_till_cursor()
{
    gfx_swap_fg_bg();
    gfx_fill_rect( 0, ctx.term.cursor_row*8, ctx.term.cursor_col*8, 8 );
    gfx_swap_fg_bg();
}


void gfx_term_clear_line()
{
    gfx_swap_fg_bg();
    gfx_fill_rect( 0, ctx.term.cursor_row*8, ctx.W, 8 );
    gfx_swap_fg_bg();
    gfx_term_render_cursor();
}


void gfx_term_clear_screen()
{
    gfx_clear();
    gfx_term_render_cursor();
}


/*
 *  Term ansii codes scanner
 *
 */
#define TERM_ESCAPE_CHAR (0x1B)


/*
 * State implementations
 */

void state_fun_final_letter( char ch, scn_state *state )
{
    if( state->private_mode_char == '#' )
    {
        // Non-standard ANSI Codes
        switch( ch )
        {
            case 'l':
                /* render line */
                if( state->cmd_params_size == 4 )
                {
                    gfx_line( state->cmd_params[0], state->cmd_params[1], state->cmd_params[2], state->cmd_params[3] );
                }
            goto back_to_normal;
            break;
            case 'r':
                /* render a filled rectangle */
                if( state->cmd_params_size == 4 )
                {
                    gfx_fill_rect( state->cmd_params[0], state->cmd_params[1], state->cmd_params[2], state->cmd_params[3] );
                }
            goto back_to_normal;
            break;
        }

    }

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

        case 'H':
            if( state->cmd_params_size == 2 )
            {
                gfx_term_move_cursor( state->cmd_params[0], state->cmd_params[1]);
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
}

void state_fun_read_digit( char ch, scn_state *state )
{
    if( ch>='0' && ch <= '9' )
    {
        // parse digit
        state->cmd_params[ state->cmd_params_size - 1] = state->cmd_params[ state->cmd_params_size - 1]*10 + (ch-'0');
        state->next = state_fun_read_digit; // stay on this state
        return;
    }

    if( ch == ';' )
    {
        // Another param will follow
        state->cmd_params_size++;
        state->cmd_params[ state->cmd_params_size-1 ] = 0;
        state->next = state_fun_read_digit; // stay on this state
        return;
    }

    // not a digit, call the final state
    state_fun_final_letter( ch, state );
}

void state_fun_selectescape( char ch, scn_state *state )
{
    if( ch>='0' && ch<='9' )
    {
        // start of a digit
        state->cmd_params_size = 1;
        state->cmd_params[ 0 ] = ch-'0';
        state->next = state_fun_read_digit;
        return;
    }
    else
    {
        if( ch=='?' || ch=='#' )
        {
            state->private_mode_char = ch;

            // A digit will follow
            state->cmd_params_size = 1;
            state->cmd_params[ 0 ] = 0;
            state->next = state_fun_read_digit;
            return;
        }
    }

    // Already at the final letter
    state_fun_final_letter( ch, state );

}

void state_fun_waitsquarebracket( char ch, scn_state *state )
{
    if( ch=='[' )
    {
        state->cmd_params[0]=1;
        state->private_mode_char=0; /* reset private mode char */
        state->next = state_fun_selectescape;
        return;
    }

    if( ch==TERM_ESCAPE_CHAR ) // Double ESCAPE prints the ESC character
    {
        gfx_putc( ctx.term.cursor_row, ctx.term.cursor_col, ch );
        ++ctx.term.cursor_col;
        gfx_term_render_cursor();
    }

    state->next = state_fun_normaltext;
}

void state_fun_normaltext( char ch, scn_state *state )
{
    if( ch==TERM_ESCAPE_CHAR )
    {
        state->next = state_fun_waitsquarebracket;
        return;
    }

    gfx_putc( ctx.term.cursor_row, ctx.term.cursor_col, ch );
    ++ctx.term.cursor_col;
    gfx_term_render_cursor();
}
