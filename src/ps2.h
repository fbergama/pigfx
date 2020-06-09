#ifndef PS2_H__
#define PS2_H__

#define PS2DATAPIN  2
#define PS2CLOCKPIN 3

#define INPUTBUFFSIZE 10
#define RECEIVETIMEOUT 20000    // 20ms


typedef struct keyboard_inout_t
{
    unsigned char keyboardType[2];
    unsigned char scanCodeSet;
    unsigned char sending;      // flag
    unsigned char sendByte;
    unsigned char sendParity;
    unsigned char bit_cnt;
    unsigned char readPos;
    unsigned char writePos;
    unsigned char fromKeyboard[INPUTBUFFSIZE];
} keyboard_inout_t;

unsigned char initPS2();
void handlePS2ClockEvent(__attribute__((unused)) void* data);
unsigned char getPS2char(unsigned char *fromKbd);
void sendPS2Byte(unsigned char sendVal);
void PS2KeyboardHandler();
void setPS2Leds(unsigned char scroll, unsigned char num, unsigned char caps);

#endif
