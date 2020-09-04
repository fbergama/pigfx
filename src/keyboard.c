//
// keyboard.c
// Keyboard input handling
//
// PiGFX is a bare metal kernel for the Raspberry Pi
// that implements a basic ANSI terminal emulator with
// the additional support of some primitive graphics functions.
// Copyright (C) 2020 Christian Lehner
// Most of this code is loosely taken from the USPI project

#include "keyboard.h"
#include "c_utils.h"
#include "config.h"
#include "uart.h"
#include "gfx.h"
#include "timer.h"
#include "ps2.h"

// order must match TSpecialKey beginning at KeySpace
static const char *s_KeyStrings[KeyMaxCode-KeySpace] =
{
	" ",			// KeySpace
	"\x1b",			// KeyEscape
	"\x7f",			// KeyBackspace
	"\t",			// KeyTabulator
	"\n",			// KeyReturn
	"\x1b[2~",		// KeyInsert
	"\x1b[1~",		// KeyHome
	"\x1b[5~",		// KeyPageUp
	"\x1b[3~",		// KeyDelete
	"\x1b[4~",		// KeyEnd
	"\x1b[6~",		// KeyPageDown
	"\x1b[A",		// KeyUp
	"\x1b[B",		// KeyDown
	"\x1b[D",		// KeyLeft
	"\x1b[C",		// KeyRight
	"\x1b[[A",		// KeyF1
	"\x1b[[B",		// KeyF2
	"\x1b[[C",		// KeyF3
	"\x1b[[D",		// KeyF4
	"\x1b[[E",		// KeyF5
	"\x1b[17~",		// KeyF6
	"\x1b[18~",		// KeyF7
	"\x1b[19~",		// KeyF8
	"\x1b[20~",		// KeyF9
	0,			// KeyF10
	0,			// KeyF11
	0,			// KeyF12
	0,			// KeyApplication
	0,			// KeyCapsLock
	0,			// KeyPrintScreen
	0,			// KeyScrollLock
	0,			// KeyPause
	0,			// KeyNumLock
	"/",			// KeyKP_Divide
	"*",			// KeyKP_Multiply
	"-",			// KeyKP_Subtract
	"+",			// KeyKP_Add
	"\n",			// KeyKP_Enter
	"1",			// KeyKP_1
	"2",			// KeyKP_2
	"3",			// KeyKP_3
	"4",			// KeyKP_4
	"5",			// KeyKP_5
	"6",			// KeyKP_6
	"7",			// KeyKP_7
	"8",			// KeyKP_8
	"9",			// KeyKP_9
	"0",			// KeyKP_0
	"\x1b[G",		// KeyKP_Center
	",",			// KeyKP_Comma
	"."			// KeyKP_Period
};

TKeyMap actKeyMap;
unsigned int backspace_n_skip;
unsigned int last_backspace_t;

void fSetKbdLeds(TKeyboardLeds* leds)
{
    leds->CombinedState = (leds->m_bNumLock << 0) | (leds->m_bCapsLock << 1) | (leds->m_bScrollLock << 2);
}

void fUpdateKeyboardLeds(unsigned char useUSB)
{
    if (actKeyMap.leds.CombinedState != actKeyMap.leds.LastCombinedState)
    {
        actKeyMap.leds.LastCombinedState = actKeyMap.leds.CombinedState;
        if (useUSB == 0) setPS2Leds(actKeyMap.leds.m_bScrollLock, actKeyMap.leds.m_bNumLock, actKeyMap.leds.m_bCapsLock);
#if RPI<4
        else USPiKeyboardSetLEDs(actKeyMap.leds.CombinedState);
#endif
    }
}

void fInitKeyboard(char* layout)
{
    backspace_n_skip = 0;
    last_backspace_t = 0;

    actKeyMap.leds.m_bCapsLock   = 0;
    actKeyMap.leds.m_bNumLock    = 1;
    actKeyMap.leds.m_bScrollLock = 0;
    actKeyMap.leds.LastCombinedState = 0;
    fSetKbdLeds(&actKeyMap.leds);

    actKeyMap.ucLastPhyCode = 0;
    actKeyMap.ucModifiers = 0;
    actKeyMap.repeatTimerHnd = 0;

    if      ((layout[0] == 'u') && (layout[1] == 'k')) pigfx_memcpy(&actKeyMap.m_KeyMap, keyMap_uk, sizeof(actKeyMap.m_KeyMap));
    else if ((layout[0] == 'i') && (layout[1] == 't')) pigfx_memcpy(&actKeyMap.m_KeyMap, keyMap_it, sizeof(actKeyMap.m_KeyMap));
    else if ((layout[0] == 'f') && (layout[1] == 'r')) pigfx_memcpy(&actKeyMap.m_KeyMap, keyMap_fr, sizeof(actKeyMap.m_KeyMap));
    else if ((layout[0] == 'e') && (layout[1] == 's')) pigfx_memcpy(&actKeyMap.m_KeyMap, keyMap_es, sizeof(actKeyMap.m_KeyMap));
    else if ((layout[0] == 'd') && (layout[1] == 'e')) pigfx_memcpy(&actKeyMap.m_KeyMap, keyMap_de, sizeof(actKeyMap.m_KeyMap));
    else if ((layout[0] == 's') && (layout[1] == 'g')) pigfx_memcpy(&actKeyMap.m_KeyMap, keyMap_sg, sizeof(actKeyMap.m_KeyMap));
    // all else is us
    else                                               pigfx_memcpy(&actKeyMap.m_KeyMap, keyMap_us, sizeof(actKeyMap.m_KeyMap));

}

unsigned short ScancodeToKey (TKeyMap *pThis, unsigned char nPhyCode, unsigned char nModifiers)
{
    if (   pThis == 0
        || nPhyCode == 0
        || nPhyCode > PHY_MAX_CODE)
    {
        return KeyNone;
    }

    unsigned short nLogCodeNorm = pThis->m_KeyMap[nPhyCode][K_NORMTAB];

    if (   nLogCodeNorm == KeyDelete
        && (nModifiers & (LCTRL | RCTRL))
        && (nModifiers & ALT))
    {
        return ActionShutdown;
    }

    if (   (KeyF1 <= nLogCodeNorm && nLogCodeNorm <= KeyF12)
        && (nModifiers & ALT))
    {
        return ActionSelectConsole1 + (nLogCodeNorm - KeyF1);
    }

    if (nModifiers & (ALT | LWIN | RWIN))
    {
        return KeyNone;
    }

    unsigned nTable = K_NORMTAB;

    // TODO: hard-wired to keypad
    if (KEYPAD_FIRST <= nPhyCode && nPhyCode <= KEYPAD_LAST)
    {
        if (pThis->leds.m_bNumLock)
        {
            nTable = K_SHIFTTAB;
        }
    }
    else if (nModifiers & ALTGR)
    {
        if (nModifiers & (LSHIFT | RSHIFT))
        {
            nTable = K_ALTSHIFTTAB;
        }
        else
        {
            nTable = K_ALTTAB;
        }
    }
    else if (nModifiers & (LSHIFT | RSHIFT))
    {
        nTable = K_SHIFTTAB;
    }

    unsigned short nLogCode = pThis->m_KeyMap[nPhyCode][nTable];

    switch (nLogCode)
    {
    case KeyCapsLock:
        pThis->leds.m_bCapsLock = !pThis->leds.m_bCapsLock;
        return ActionSwitchCapsLock;

    case KeyNumLock:
        pThis->leds.m_bNumLock = !pThis->leds.m_bNumLock;
        return ActionSwitchNumLock;

    case KeyScrollLock:
        pThis->leds.m_bScrollLock = !pThis->leds.m_bScrollLock;
        return ActionSwitchScrollLock;
    }

    return nLogCode;
}

const char *KeyToString (TKeyMap *pThis, unsigned short nKeyCode, unsigned char nModifiers, char Buffer[2])
{
    if (   pThis == 0
        || nKeyCode <= ' '
        || nKeyCode >= KeyMaxCode)
    {
        return 0;
    }

    if (KeySpace <= nKeyCode && nKeyCode < KeyMaxCode)
    {
        return s_KeyStrings[nKeyCode-KeySpace];
    }

    char chChar = (char) nKeyCode;

    if (nModifiers & (LCTRL | RCTRL))
    {
        chChar -= 'a';
        if (('\0' <= chChar) && (chChar <= 'z'-'a'))
        {
            Buffer[0] = chChar + 1;
            Buffer[1] = '\0';

            return Buffer;
        }

        return 0;
    }

    if (pThis->leds.m_bCapsLock)
    {
        if ('A' <= chChar && chChar <= 'Z')
        {
            chChar += 'a'-'A';
        }
        else if ('a' <= chChar && chChar <= 'z')
        {
            chChar -= 'a'-'A';
        }
    }

    Buffer[0] = chChar;
    Buffer[1] = '\0';

    return Buffer;
}

void KeyEvent(unsigned short ucKeyCode, unsigned char ucModifiers)
{
    const char* pKeyString = 0;
    char buffer[2];

    unsigned short key = ScancodeToKey(&actKeyMap, ucKeyCode, ucModifiers);

    switch (key)
    {
    case ActionSwitchCapsLock:
    case ActionSwitchNumLock:
    case ActionSwitchScrollLock:
        fSetKbdLeds(&actKeyMap.leds);
        break;

    case ActionSelectConsole1:
    case ActionSelectConsole2:
    case ActionSelectConsole3:
    case ActionSelectConsole4:
    case ActionSelectConsole5:
    case ActionSelectConsole6:
    case ActionSelectConsole7:
    case ActionSelectConsole8:
    case ActionSelectConsole9:
    case ActionSelectConsole10:
    case ActionSelectConsole11:
    case ActionSelectConsole12:
        // unused
        break;

    case ActionShutdown:
        // unused
        break;

    default:
        pKeyString = KeyToString (&actKeyMap, key, ucModifiers, buffer);
        if (pKeyString != 0)
        {
            const char* c = pKeyString;
            char CR = 13;

            while( *c )
            {
                char ch = *c;

                if ((PiGfxConfig.sendCRLF) && (ch == 10))
                {
                    // Send CR first
                    uart_write( CR );
                }

                if ((PiGfxConfig.replaceLFwithCR) && (ch == 10))
                {
                    ch = CR;
                }

                if ((PiGfxConfig.swapDelWithBackspace) && (ch == 0x7F))
                {
                    ch = 0x8;
                }

                if ((PiGfxConfig.backspaceEcho) && (ch == 0x8))
                    gfx_term_putstring( "\x7F" );

                if ((PiGfxConfig.skipBackspaceEcho) && (ch == 0x7F))
                {
                    backspace_n_skip = 2;
                    last_backspace_t = time_microsec();
                }

                uart_write( ch );
                ++c;
            }
        }
        break;
    }
}

void RepeatKey( unsigned hnd, void* pParam, void *pContext )
{
    (void)hnd;
    (void)pContext;
    TKeyMap* p = (TKeyMap*)pParam;

    if (p->ucLastPhyCode != 0)
    {
        KeyEvent(p->ucLastPhyCode, p->ucModifiers);
        p->repeatTimerHnd = attach_timer_handler(REPEAT_FREQ, RepeatKey, p, 0);       // 100ms interval
    }
}

void KeyStatusHandlerRaw (unsigned char ucModifiers, const unsigned char RawKeys[6])  // key code or 0 in each byte
{
    unsigned char ucKeyCode = 0;

    for (int i = 5; i >= 0; i--)
    {
        ucKeyCode = RawKeys[i];
        if (ucKeyCode != 0)
        {
            break;
        }
    }
    if (ucKeyCode == 1)
    {
        // too many keys pressed
        return;
    }

    if (ucKeyCode == actKeyMap.ucLastPhyCode) return;
    else actKeyMap.ucLastPhyCode = ucKeyCode;

    actKeyMap.ucModifiers = ucModifiers;

    if (ucKeyCode)
    {
        KeyEvent(ucKeyCode, ucModifiers);

        if (actKeyMap.repeatTimerHnd) remove_timer(actKeyMap.repeatTimerHnd);

        actKeyMap.repeatTimerHnd = attach_timer_handler(1, RepeatKey, (void*)&actKeyMap, 0);       // 1 until repeat starts
    }
    else
    {
        if (actKeyMap.repeatTimerHnd) {
            remove_timer(actKeyMap.repeatTimerHnd);
            actKeyMap.repeatTimerHnd = 0;
        }
    }

}

