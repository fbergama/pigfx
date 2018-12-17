/*
 * scn_state.h
 *
 * Types and structures for the ANSI sequence parser states.
 * This has been extracted from gfx.c
 *
 *  Created on: 6 nov. 2018
 *      Author: Francis Pierot
 */

#ifndef SRC_SCN_STATE_H_
#define SRC_SCN_STATE_H_


/** Forward declarations needed for function type used in scn_state structure. */
struct SCN_STATE;
typedef struct SCN_STATE scn_state;

/** Function type for scanning state.
 *	@param ch the character to scan
 *	@param state points to the current state structure
 *	@return true if the terminal should handle line break and screen scroll returning from this call.
 */
typedef int state_fun( char ch, scn_state *state );

/** Scanning state. Contains the status of the input sequence scanning. */
typedef struct SCN_STATE
{
	/*
	 * Parsing functions
	 */
    state_fun (*next);
    state_fun (*after_read_digit);

    /*
     * ANSI term mode (normal ESC[ sequences and ANSI private mode sequences)
     */
    unsigned int cmd_params[10];	// Scanned parameters after escape sequence start
    unsigned int cmd_params_size;	// Number off parameters after escape sequence start
    char private_mode_char;			// Escape private mode character (ESC followed by not '[')

    /*
     * TODO: Graphic Extension mode (private mode GPH8 or GPH16)
     *
    unsigned int gph_params_size;	// 8 for 1-byte coordinates, 16 for 2-bytes
    unsigned int gph_commandcode;	// 0-255
    */
} scn_state;



#endif /* SRC_SCN_STATE_H_ */
