#include "hexloader.h"
#include "libfs.h"
#include "gfx.h"
#include "ee_printf.h"
#include "nmalloc.h"
#include "utils.h"
#include "uart.h"
#include "buffered_uart.h"
#include "timer.h"


#define HEXLOADER_FG_COL 7
#define HEXLOADER_BG_COL 12
#define HEXLOADER_W 48
#define HEXLOADER_H 24
#define QUOTE(...) #__VA_ARGS__


typedef struct
{
    unsigned int x;
    unsigned int y;
    unsigned int width;
    unsigned int height;

    unsigned int term_r0;
    unsigned int term_c0;

} Hexloader_win;


static Hexloader_win win;
static struct dirent* path[32];
static unsigned int path_idx=0;
static unsigned int esc_status;
static struct dirent* selected;
static unsigned int scroll_start;
static unsigned int scroll_end;
static unsigned int selected_idx;
static unsigned char active=0;


static const char* bootloader = QUOTE(
NEW\r\n
CLEAR\r\n
10 REM by FB and DaveP\r\n
20 REM V0.01\r\n
30 PRINT "Loading"\r\n
40 LET MB = &HF800\r\n
50 PRINT "Start:", HEX$(MB)\r\n
60 REM Go to READ Sub.\r\n
70 GOSUB 1000\r\n
80 PRINT "End:", HEX$(MB-1)\r\n
90 REM Change Pointer for USR\r\n
100 GOSUB 1100\r\n
110 REM RUN!\r\n
120 PRINT USR(0)\r\n
130 END\r\n
1000 REM load routine\r\n
1010 REM Needs var mb set to start loc\r\n
1020 READ A\r\n
1030 IF A > 255 THEN RETURN\r\n
1040 REM print HEX$(mb), a\r\n
1050 POKE MB, A\r\n
1060 LET MB = MB + 1\r\n
1070 GOTO 1020\r\n
1100 REM    Loc of ptr &H8049\r\n
1110 PRINT "Jump Loc"\r\n
1120 LET MB = &H8048\r\n
1130 REM    JP start addr (c3 00 f8) jp f800\r\n
1140 POKE MB, &HC3\r\n
1150 POKE MB + 1, &H00\r\n
1160 POKE MB + 2, &HF8\r\n
1170 RETURN\r\n
9010 DATA 33, 255, 255, 249, 62, 3, 211, 128, 62, 150, 211, 128\r\n
9040 DATA 251, 33, 200, 248, 205, 140, 248, 205, 135, 248, 125, 254\r\n
9070 DATA 58, 194, 19, 248, 221, 33, 0, 0, 205, 150, 248, 68\r\n
9100 DATA 77, 205, 150, 248, 85, 205, 150, 248, 93, 205, 150, 248\r\n
9130 DATA 125, 254, 1, 40, 36, 254, 0, 32, 45, 205, 150, 248\r\n
9160 DATA 125, 18, 19, 11, 62, 0, 176, 177, 254, 0, 32, 241\r\n
9190 DATA 205, 150, 248, 221, 125, 254, 0, 32, 30, 62, 42, 205\r\n
9220 DATA 124, 248, 195, 19, 248, 205, 150, 248, 33, 255, 248, 205\r\n
9250 DATA 140, 248, 33, 128, 128, 233, 33, 235, 248, 205, 140, 248\r\n
9280 DATA 195, 120, 248, 33, 245, 248, 205, 140, 248, 195, 120, 248\r\n
9310 DATA 0, 195, 120, 248, 245, 219, 128, 203, 79, 40, 250, 241\r\n
9340 DATA 211, 129, 201, 245, 215, 111, 241, 201, 126, 183, 200, 205\r\n
9370 DATA 124, 248, 35, 195, 140, 248, 245, 213, 205, 135, 248, 125\r\n
9400 DATA 214, 48, 254, 10, 56, 2, 214, 7, 87, 205, 135, 248\r\n
9430 DATA 125, 214, 48, 254, 10, 56, 2, 214, 7, 95, 203, 34\r\n
9460 DATA 203, 34, 203, 34, 203, 34, 178, 209, 38, 0, 111, 241\r\n
9490 DATA 197, 6, 0, 77, 221, 9, 193, 201, 72, 69, 88, 32\r\n
9520 DATA 76, 79, 65, 68, 69, 82, 32, 98, 121, 32, 70, 105\r\n
9550 DATA 108, 105, 112, 112, 111, 32, 66, 101, 114, 103, 97, 109\r\n
9580 DATA 97, 115, 99, 111, 10, 13, 0, 10, 13, 73, 78, 86\r\n
9610 DATA 32, 84, 89, 80, 0, 10, 13, 66, 65, 68, 32, 67\r\n
9640 DATA 72, 75, 0, 10, 13, 79, 75, 0, 0, 0, 0, 0\r\n
9670 DATA 999\r\n
9999 END\r\n
RUN\r\n
);


static char* build_path( char* pathstr )
{
    char* orig_pathstr = pathstr;
    unsigned int i;
    char* dname;

    *pathstr++ = '/';

    // Copy directory path
    for( i=0; i<path_idx; ++i )
    {
        dname = path[i]->name;
        while( *dname )
            *pathstr++ = *dname++;

        *pathstr++ = '/';
    }

    if( selected )
    {
        // Copy filename
        dname = selected->name;
        while( *dname )
            *pathstr++ = *dname++;
    }

    *pathstr++ = 0; // end of string
    return orig_pathstr;
}


static void clear_list( struct dirent* l )
{
    while( l && l->next )
    {
        if( l->next->name[0]=='_' || l->next->name[0]=='.' || l->next->name[0]=='~' )
            l->next = l->next->next;

        l = l->next;
    }
}


static void hexloader_set_status( const char* str )
{
    gfx_term_move_cursor( win.term_r0+20, win.term_c0+HEXLOADER_W-17 );
    ee_printf("%s",str);
}


static void hexloader_show_progress( int currv, int maxv )
{
    unsigned int nblocks = (currv<<4)/maxv;
    unsigned int i;

    gfx_term_move_cursor( win.term_r0+22, win.term_c0+HEXLOADER_W-17 );

    for( i=0; i<16; ++i)
    {
        if( i<nblocks )
            gfx_set_bg( HEXLOADER_FG_COL );
        else
            gfx_set_bg( HEXLOADER_BG_COL );

        ee_printf(" ");
    }

    gfx_set_bg( HEXLOADER_BG_COL );
}


static void uart_slow_write_str( const char* data, unsigned int ch_delay, unsigned int line_delay )
{
    const char* pData = data;
    unsigned int nchars = strlen( data );
    unsigned int i=0;

    while( *pData )
    {
        uart_write( pData, 1 );
        usleep( ch_delay );

        if( *pData == '\r' || *pData == '\n' )
            usleep( line_delay );

        hexloader_show_progress( i, nchars );
        ++i;
        ++pData;
    }
}


void hexloader_draw_window()
{
    char buff[ HEXLOADER_W +1 ] = {0};
    unsigned int idx=0;

    gfx_set_fg( HEXLOADER_FG_COL );
    gfx_set_bg( HEXLOADER_BG_COL );
    gfx_term_set_cursor_visibility(0);

    /* Top line */
    buff[idx++]=24;
    while( idx<HEXLOADER_W-1 )
        buff[idx++]=23;
    buff[idx++]=25;
    gfx_term_move_cursor( win.term_r0, win.term_c0 );
    gfx_term_putstring( buff );

    /* Bottom line */
    buff[0]=26;
    buff[HEXLOADER_W-1]=0;

    gfx_term_move_cursor( win.term_r0+HEXLOADER_H, win.term_c0 );
    gfx_term_putstring( buff );

    /* left-right lines */
    idx=0;
    buff[idx++]=22;
    while( idx<HEXLOADER_W-1 )
        buff[idx++]=' ';
    buff[HEXLOADER_W-19]=22;
    buff[idx++]=22;

    for( idx=1; idx<HEXLOADER_H; ++idx )
    {
        gfx_term_move_cursor( win.term_r0+idx, win.term_c0 );
        gfx_term_putstring( buff );
    }

    gfx_putc( win.term_r0+HEXLOADER_H, win.term_c0+HEXLOADER_W-1, 27);


    /* print help */
    gfx_term_move_cursor( win.term_r0+2, win.term_c0+HEXLOADER_W-17 );
    ee_printf("    RC2014");
    gfx_term_move_cursor( win.term_r0+3, win.term_c0+HEXLOADER_W-17 );
    ee_printf("  HEXLOADER");
    gfx_term_move_cursor( win.term_r0+4, win.term_c0+HEXLOADER_W-17 );
    ee_printf("    %c%c%c%c%c%c ",23,23,23,23,23,23);
    gfx_term_move_cursor( win.term_r0+7, win.term_c0+HEXLOADER_W-17 );
    ee_printf("Commands:");
    gfx_term_move_cursor( win.term_r0+9, win.term_c0+HEXLOADER_W-17 );
    ee_printf("%c %c   Move",205,206);
    gfx_term_move_cursor( win.term_r0+11, win.term_c0+HEXLOADER_W-17 );
    ee_printf("ENTER Load HEX",205,206);
    gfx_term_move_cursor( win.term_r0+13, win.term_c0+HEXLOADER_W-17 );
    ee_printf("%c    Parent dir",208);
    gfx_term_move_cursor( win.term_r0+15, win.term_c0+HEXLOADER_W-17 );
    ee_printf(" %c   Enter dir",207);
    gfx_term_move_cursor( win.term_r0+17, win.term_c0+HEXLOADER_W-17 );
    ee_printf("Q     Quit",205,206);
}


void hexloader_list()
{
    unsigned int curr_row = win.term_r0+2;
    unsigned int curr_col = win.term_c0+2;
    struct dirent* entry = path[path_idx];
    unsigned int idx=0;

    while( entry != 0 )
    {
        if( idx>=scroll_start && idx<scroll_end )
        {
            gfx_term_move_cursor( curr_row, curr_col );
            ee_printf("                    ");

            if( entry==selected )
            {
                gfx_set_fg( HEXLOADER_BG_COL );
                gfx_set_bg( HEXLOADER_FG_COL );
            }
            gfx_term_move_cursor( curr_row, curr_col );
            ee_printf("%s", entry->name);

            gfx_term_move_cursor( curr_row, curr_col+14 );
            gfx_set_fg( HEXLOADER_FG_COL );
            gfx_set_bg( HEXLOADER_BG_COL );

            if( entry->is_dir )
                ee_printf("DIR");
            else if( entry->byte_size > (1<<20) )
                ee_printf("%6d Mb", entry->byte_size >> 20);
            else if( entry->byte_size > 1024 )
                ee_printf("%6d Kb", entry->byte_size >> 10);
            else
                ee_printf("%6d bytes", entry->byte_size);

            ++curr_row;
        }

        entry = entry->next;
        ++idx;
    }
}


void hexloader_show( )
{
    unsigned int term_r;
    unsigned int term_c;

    esc_status = 0;
    path_idx=0;
    scroll_start=0;
    scroll_end=HEXLOADER_H-3;
    selected_idx=0;

    DIR* newd = opendir( "/" );
    path[path_idx] = readdir( newd );
    clear_list( path[path_idx] );

    selected = path[path_idx];

    gfx_get_term_size( &term_r, &term_c );

    win.term_c0 = ((term_c-HEXLOADER_W)>>1);
    win.term_r0 = ((term_r-HEXLOADER_H)>>1);
    hexloader_draw_window();
    hexloader_list();
    active = 1;
}


void hexloader_destroy()
{
    active=0;
    gfx_set_fg(15);
    gfx_set_bg(0);
    gfx_clear();
}


unsigned char hexloader_keypressed( unsigned int keycode )
{
    if( !active )
        return active;

    struct dirent* entry = path[ path_idx ];
    switch( keycode )
    {
        case 0x1B:
            // escape seq
            esc_status = 1;
            break;

        case 0x5B:
            // escape seq
            if( esc_status == 1 )
                esc_status = 2;
            else
                esc_status = 0;
            break;

        case 0x42:
            if( esc_status != 2 )
                break;
            esc_status=0;

            // down arrow
            if( selected && selected->next )
            {
                selected = selected->next;
                selected_idx++;
                if( selected_idx >= scroll_end )
                {
                    scroll_start++;
                    scroll_end++;
                }
            }

            break;

        case 0x41:
            // up arrow
            if( esc_status != 2 )
                break;
            esc_status=0;

            entry = path[path_idx];
            while( entry != 0 )
            {
                if( entry->next == selected )
                {
                    selected = entry;
                    selected_idx--;
                    if( selected_idx < scroll_start )
                    {
                        scroll_start--;
                        scroll_end--;
                    }
                    break;
                }
                entry = entry->next;
            }
            break;

        case 0x44:
            // left arrow
            if( path_idx > 0 )
            {
                char dirpath[256];
                --path_idx;
                selected = 0;
                build_path(dirpath);
                DIR* newd = opendir( build_path( dirpath ) );
                path[path_idx] = readdir( newd );
                clear_list( path[path_idx] );
                selected = path[path_idx];
                scroll_start=0;
                selected_idx=0;
                scroll_end=HEXLOADER_H-3;
                hexloader_draw_window();
            }

            break;

        case 0x43:
            // right arrow
            {
                // Enter the selected directory
                char dirpath[256];
                build_path(dirpath);
                DIR* newd = opendir( build_path( dirpath ) );
                path[path_idx]=selected;
                path[++path_idx] = readdir( newd );
                clear_list( path[path_idx] );
                selected = path[path_idx];
                scroll_start=0;
                selected_idx=0;
                scroll_end=HEXLOADER_H-3;
                hexloader_draw_window();
            }
            break;

        case 10:
            {

                if( !selected->is_dir )
                {
                    // Load the specified file
                    FILE* f=0;
                    char filepath[256];
                    char *file_buffer = nmalloc_malloc( selected->byte_size+1 );

                    uart_purge();
                    buart_purge();

                    build_path( filepath );
                    hexloader_set_status("Loading BAS");
                    uart_slow_write_str( bootloader, 5000, 20000 );
                    f = fopen( filepath, "r" );
                    if( f )
                    {
                        fread( file_buffer, selected->byte_size, 1, f );
                        file_buffer[selected->byte_size] = 0;

                        usleep( 3000000 ); // Give some time to the hexloader to start

                        hexloader_set_status("Loading HEX");
                        uart_slow_write_str( file_buffer, 1000, 5000 );
                        nmalloc_free( (void**)&file_buffer );
                        hexloader_destroy();
                    }

                    uart_purge();
                    buart_purge();
                    return active;
                }
                break;
            }

        case 'q':
            hexloader_destroy();
            return active;

    }

    hexloader_list();
    return active;
}

