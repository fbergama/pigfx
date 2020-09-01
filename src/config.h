//
// config.c
// Read the content of a ini file and set the configuration
//
// PiGFX is a bare metal kernel for the Raspberry Pi
// that implements a basic ANSI terminal emulator with
// the additional support of some primitive graphics functions.
// Copyright (C) 2020 Christian Lehner

#ifndef CONFIG_H_
#define CONFIG_H_

#define CONFIGFILENAME     "pigfx.txt"

#define errOK           0
#define errSDCARDINIT   1
#define errMBR          2
#define errFS           3
#define errREADROOT     4
#define errLOCFILE      5
#define errOPENFILE     6
#define errREADFILE     7
#define errSYNTAX       8


typedef struct
{
    unsigned int uartBaudrate;          // The desired baudrate of the UART interface
    unsigned int useUsbKeyboard;        // Use uspi to enable a USB keyboard
    unsigned int sendCRLF;              // send CRLF instead of only LF
    unsigned int replaceLFwithCR;       // Send CR instead of LF
    unsigned int backspaceEcho;         // Auto-echo the backspace char
    unsigned int skipBackspaceEcho;     // Skip the next incoming character after a backspace from keyboard
    unsigned int swapDelWithBackspace;  // Substitute DEL (0x7F) with BACKSPACE (0x08)
    unsigned int showRC2014Logo;        // Show the RC2014 logo at startup
    unsigned int disableGfxDMA;         // Disable DMA for Gfx if 1
    unsigned int disableCollision;      // Disable collision detection if 1
    char         keyboardLayout[3];     // Keyboard layout (de,uk,us, ...)
} tPiGfxConfig;

extern tPiGfxConfig PiGfxConfig;


void setDefaultConfig();
unsigned char lookForConfigFile();

#endif
