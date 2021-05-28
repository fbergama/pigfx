//
// keyboard.h
// Keyboard input handling
//
// PiGFX is a bare metal kernel for the Raspberry Pi
// that implements a basic ANSI terminal emulator with
// the additional support of some primitive graphics functions.
// Copyright (C) 2020 Christian Lehner
// Most of this code is loosely taken from the USPI project

#ifndef KEYBOARD_H_
#define KEYBOARD_H_

typedef unsigned short u16;
typedef unsigned char   u8;

#include "../uspi/include/uspi.h"

#define REPEAT_FREQ     10       // 10 hz -> 100ms

#define PHY_MAX_CODE    127

#define KEYPAD_FIRST    0x53
#define KEYPAD_LAST     0x63

#define K_NORMTAB       0
#define K_SHIFTTAB      1
#define K_ALTTAB        2
#define K_ALTSHIFTTAB   3

#define C(chr)      ((u16) (u8) (chr))

typedef enum
{
	KeyNone  = 0x00,
	KeySpace = 0x100,
	KeyEscape,
	KeyBackspace,
	KeyTabulator,
	KeyReturn,
	KeyInsert,
	KeyHome,
	KeyPageUp,
	KeyDelete,
	KeyEnd,
	KeyPageDown,
	KeyUp,
	KeyDown,
	KeyLeft,
	KeyRight,
	KeyF1,
	KeyF2,
	KeyF3,
	KeyF4,
	KeyF5,
	KeyF6,
	KeyF7,
	KeyF8,
	KeyF9,
	KeyF10,
	KeyF11,
	KeyF12,
	KeyApplication,
	KeyCapsLock,
	KeyPrintScreen,
	KeyScrollLock,
	KeyPause,
	KeyNumLock,
	KeyKP_Divide,
	KeyKP_Multiply,
	KeyKP_Subtract,
	KeyKP_Add,
	KeyKP_Enter,
	KeyKP_1,
	KeyKP_2,
	KeyKP_3,
	KeyKP_4,
	KeyKP_5,
	KeyKP_6,
	KeyKP_7,
	KeyKP_8,
	KeyKP_9,
	KeyKP_0,
	KeyKP_Center,
	KeyKP_Comma,
	KeyKP_Period,
	KeyMaxCode
}
TSpecialKey;

typedef enum
{
	ActionSwitchCapsLock = KeyMaxCode,
	ActionSwitchNumLock,
	ActionSwitchScrollLock,
	ActionSelectConsole1,
	ActionSelectConsole2,
	ActionSelectConsole3,
	ActionSelectConsole4,
	ActionSelectConsole5,
	ActionSelectConsole6,
	ActionSelectConsole7,
	ActionSelectConsole8,
	ActionSelectConsole9,
	ActionSelectConsole10,
	ActionSelectConsole11,
	ActionSelectConsole12,
	ActionShutdown,
	ActionSetBaudrate,
	ActionNone
}
TSpecialAction;

typedef struct TKeyboardLeds
{
	unsigned char m_bCapsLock;
	unsigned char m_bNumLock;
	unsigned char m_bScrollLock;
    unsigned char CombinedState;
    unsigned char LastCombinedState;
} TKeyboardLeds;

typedef struct TKeyMap
{
	unsigned short m_KeyMap[PHY_MAX_CODE+1][K_ALTSHIFTTAB+1];
    TKeyboardLeds leds;
    unsigned char ucLastPhyCode;
    unsigned char ucModifiers;
    unsigned repeatTimerHnd;
}
TKeyMap;

static const unsigned short keyMap_de[PHY_MAX_CODE+1][K_ALTSHIFTTAB+1] =
{
    #include "../uspi/lib/keymap_de.h"
};
static const unsigned short keyMap_es[PHY_MAX_CODE+1][K_ALTSHIFTTAB+1] =
{
    #include "../uspi/lib/keymap_es.h"
};
static const unsigned short keyMap_fr[PHY_MAX_CODE+1][K_ALTSHIFTTAB+1] =
{
    #include "../uspi/lib/keymap_fr.h"
};
static const unsigned short keyMap_it[PHY_MAX_CODE+1][K_ALTSHIFTTAB+1] =
{
    #include "../uspi/lib/keymap_it.h"
};
static const unsigned short keyMap_uk[PHY_MAX_CODE+1][K_ALTSHIFTTAB+1] =
{
    #include "../uspi/lib/keymap_uk.h"
};
static const unsigned short keyMap_us[PHY_MAX_CODE+1][K_ALTSHIFTTAB+1] =
{
    #include "../uspi/lib/keymap_us.h"
};
static const unsigned short keyMap_sg[PHY_MAX_CODE+1][K_ALTSHIFTTAB+1] =
{
    #include "keymap_sg.h"
};

extern unsigned int backspace_n_skip;
extern unsigned int last_backspace_t;

void KeyStatusHandlerRaw (unsigned char ucModifiers, const unsigned char RawKeys[6]);
void fInitKeyboard(char* layout);
void fUpdateKeyboardLeds(unsigned char useUSB);
void KeyEvent(unsigned short ucKeyCode, unsigned char ucModifiers);

#endif
