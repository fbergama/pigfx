#ifndef CONFIG_H_
#define CONFIG_H_

#define CONFIGFILENAME     "pigfx.txt"

#define defUARTBAUDRATE    115200
#define defUSEUSBKEYBOARD       1

#define errOK           0
#define errSDCARDINIT   1
#define errMBR          2
#define errFS           3
#define errREADROOT     4
#define errLOCFILE      5
#define errOPENFILE     6


typedef struct
{
    unsigned int uartBaudrate;          // The desired baudrate of the UART interface
    unsigned int useUsbKeyboard;        // Use uspi to enable a USB keyboard
} tPiGfxConfig;

extern tPiGfxConfig PiGfxConfig;


void setDefaultConfig();
unsigned char lookForConfigFile();

#endif
