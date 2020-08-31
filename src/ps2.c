/* Copyright 2020 Christian Lehner
 * Based on information from this document: https://www.avrfreaks.net/sites/default/files/PS2%20Keyboard.pdf
 * and https://wiki.osdev.org/PS/2_Keyboard
 */

#include "gpio.h"
#include "irq.h"
#include "ee_printf.h"
#include "timer.h"
#include "utils.h"
#include "keyboard.h"
#include "ps2.h"

// PS/2 Data
#define PS2_SETLEDS         0xED
#define PS2_ECHO            0xEE
#define PS2_GETSETSCANCODE  0xF0
#define PS2_IDENTIFYDEV     0xF2
#define PS2_RATEANDDELAY    0xF3
#define PS2_ENABLESCANNING  0xF4
#define PS2_DISABLESCANNING 0xF5
#define PS2_ACK             0xFA
#define PS2_RESEND          0xFE

keyboard_inout_t inout;
unsigned char keyModifiers = 0;     // see uspi ucModifiers

typedef enum ps2ToUsbState_t
{
    ps2_idle,
    ps2_E0,
    ps2_E1
} ps2ToUsbState_t;

ps2ToUsbState_t ps2ToUsbState;
unsigned char ps2E1CntChars = 0;
unsigned char ps2E0released = 0;
unsigned char ps2released = 0;

static const unsigned char ps2_set2_to_usb_map[] = {
  [0x00] = 0x00,        // nothing
  [0x01] = 0x42,        // F9
  [0x02] = 0x00,        // nothing
  [0x03] = 0x3e,        // F5
  [0x04] = 0x3c,        // F3
  [0x05] = 0x3a,        // F1
  [0x06] = 0x3b,        // F2
  [0x07] = 0x45,        // F12
  [0x08] = 0x00,        // nothing
  [0x09] = 0x43,        // F10
  [0x0a] = 0x41,        // F8
  [0x0b] = 0x3f,        // F6
  [0x0c] = 0x3d,        // F4
  [0x0d] = 0x2b,        // TAB
  [0x0e] = 0x35,        // back tick `
  [0x0f] = 0x00,        // nothing
  [0x10] = 0x00,        // nothing
  [0x11] = 0xe2,        // left ALT
  [0x12] = 0xe1,        // left SHIFT
  [0x13] = 0x00,        // nothing
  [0x14] = 0xe0,        // left CONTROL
  [0x15] = 0x14,        // Q
  [0x16] = 0x1e,        // 1
  [0x17] = 0x00,        // nothing
  [0x18] = 0x00,        // nothing
  [0x19] = 0x00,        // nothing
  [0x1a] = 0x1d,        // Z
  [0x1b] = 0x16,        // S
  [0x1c] = 0x04,        // A
  [0x1d] = 0x1a,        // W
  [0x1e] = 0x1f,        // 2
  [0x1f] = 0x00,        // nothing
  [0x20] = 0x00,        // nothing
  [0x21] = 0x06,        // C
  [0x22] = 0x1b,        // X
  [0x23] = 0x07,        // D
  [0x24] = 0x08,        // E
  [0x25] = 0x21,        // 4
  [0x26] = 0x20,        // 3
  [0x27] = 0x00,        // nothing
  [0x28] = 0x00,        // nothing
  [0x29] = 0x2c,        // SPACE
  [0x2a] = 0x19,        // V
  [0x2b] = 0x09,        // F
  [0x2c] = 0x17,        // T
  [0x2d] = 0x15,        // R
  [0x2e] = 0x22,        // 5
  [0x2f] = 0x00,        // nothing
  [0x30] = 0x00,        // nothing
  [0x31] = 0x11,        // N
  [0x32] = 0x05,        // B
  [0x33] = 0x0b,        // H
  [0x34] = 0x0a,        // G
  [0x35] = 0x1c,        // Y
  [0x36] = 0x23,        // 6
  [0x37] = 0x00,        // nothing
  [0x38] = 0x00,        // nothing
  [0x39] = 0x00,        // nothing
  [0x3a] = 0x10,        // M
  [0x3b] = 0x0d,        // J
  [0x3c] = 0x18,        // U
  [0x3d] = 0x24,        // 7
  [0x3e] = 0x25,        // 8
  [0x3f] = 0x00,        // nothing
  [0x40] = 0x00,        // nothing
  [0x41] = 0x36,        // , and <
  [0x42] = 0x0e,        // K
  [0x43] = 0x0c,        // I
  [0x44] = 0x12,        // O
  [0x45] = 0x27,        // 0
  [0x46] = 0x26,        // 9
  [0x47] = 0x00,        // nothing
  [0x48] = 0x00,        // nothing
  [0x49] = 0x37,        // . and >
  [0x4a] = 0x38,        // / and ?
  [0x4b] = 0x0f,        // L
  [0x4c] = 0x33,        // ; and :
  [0x4d] = 0x13,        // P
  [0x4e] = 0x2d,        // - and _
  [0x4f] = 0x00,        // nothing
  [0x50] = 0x00,        // nothing
  [0x51] = 0x00,        // nothing
  [0x52] = 0x34,        // ' and "
  [0x53] = 0x00,        // nothing
  [0x54] = 0x2f,        // [ and {
  [0x55] = 0x2e,        // = and +
  [0x56] = 0x00,        // nothing
  [0x57] = 0x00,        // nothing
  [0x58] = 0x39,        // CAPS LOCK
  [0x59] = 0xe5,        // RIGHT SHIFT
  [0x5a] = 0x28,        // RETURN
  [0x5b] = 0x30,        // ] and }
  [0x5c] = 0x00,        // nothing
  [0x5d] = 0x31,        // \ and |
  [0x5e] = 0x00,        // nothing
  [0x5f] = 0x00,        // nothing
  [0x61] = 0x64,        // nothing on US, but something on other keyboards
  [0x62] = 0x00,        // nothing
  [0x63] = 0x00,        // nothing
  [0x64] = 0x00,        // nothing
  [0x65] = 0x00,        // nothing
  [0x66] = 0x2a,        // BACKSPACE
  [0x67] = 0x00,        // nothing
  [0x68] = 0x00,        // nothing
  [0x69] = 0x59,        // Keypad 1 end
  [0x6a] = 0x00,        // nothing
  [0x6b] = 0x5c,        // Keypad 4 left
  [0x6c] = 0x5f,        // Keypad 7 home
  [0x6d] = 0x00,        // nothing
  [0x6e] = 0x00,        // nothing
  [0x6f] = 0x00,        // nothing
  [0x70] = 0x62,        // Keypad 0 ins
  [0x71] = 0x63,        // Keypad . del
  [0x72] = 0x5a,        // Keypad 2 down
  [0x73] = 0x5d,        // Keypad 5
  [0x74] = 0x5e,        // Keypad 6 right
  [0x75] = 0x60,        // Keypad 8 up
  [0x76] = 0x29,        // ESC
  [0x77] = 0x53,        // NUM LOCK
  [0x78] = 0x44,        // F11
  [0x79] = 0x57,        // Keypad +
  [0x7a] = 0x5b,        // Keypad 3 PgDn
  [0x7b] = 0x56,        // Keypad -
  [0x7c] = 0x55,        // Keypad *
  [0x7d] = 0x61,        // Keypad 9 PgUp
  [0x7e] = 0x47,        // SCROLL LOCK
  [0x7f] = 0x00,        // nothing
  [0x80] = 0x00,        // nothing
  [0x81] = 0x00,        // nothing
  [0x82] = 0x00,        // nothing
  [0x83] = 0x40         // F7
};

static const unsigned char ps2_set2_e0_to_usb_map[] = {
  [0x10] = 0x00,        // (multimedia) WWW search
  [0x11] = 0xe6,        // RIGHT ALT
  [0x12] = 0x00,        // first half of print screen
  [0x14] = 0xe4,        // RIGHT CONTROL
  [0x15] = 0x00,        // (multimedia) previous track
  [0x17] = 0x00,        // some strange mode key
  [0x18] = 0x00,        // (multimedia) WWW favourites
  [0x1f] = 0xe3,        // LEFT GUI
  [0x20] = 0x00,        // (multimedia) WWW refresh
  [0x21] = 0x00,        // (multimedia) volume down
  [0x23] = 0x00,        // (multimedia) mute
  [0x27] = 0xe7,        // RIGHT GUI
  [0x28] = 0x00,        // (multimedia) WWW stop
  [0x2b] = 0x00,        // (multimedia) calculator
  [0x2f] = 0x76,        // apps
  [0x30] = 0x00,        // (multimedia) WWW forward
  [0x32] = 0x00,        // (multimedia) volume up
  [0x34] = 0x00,        // (multimedia) play/pause
  [0x37] = 0x00,        // (ACPI) power
  [0x38] = 0x00,        // (multimedia) WWW back
  [0x3a] = 0x00,        // (multimedia) WWW home
  [0x3b] = 0x00,        // (multimedia) stop
  [0x3f] = 0x00,        // (ACPI) sleep
  [0x40] = 0x00,        // (multimedia) my computer
  [0x48] = 0x00,        // (multimedia) email
  [0x4a] = 0x54,        // Keypad /
  [0x4d] = 0x00,        // (multimedia) next track
  [0x50] = 0x00,        // (multimedia) media select
  [0x5a] = 0x58,        // Keypad ENTER
  [0x5e] = 0x00,        // (ACPI) wake
  [0x69] = 0x4d,        // END
  [0x6b] = 0x50,        // LEFT ARROW
  [0x6c] = 0x4a,        // HOME
  [0x70] = 0x49,        // INSERT
  [0x71] = 0x4c,        // DELETE
  [0x72] = 0x51,        // DOWN ARROW
  [0x74] = 0x4f,        // RIGHT ARROW
  [0x75] = 0x52,        // UP ARROW
  [0x7a] = 0x4e,        // PAGE DOWN
  [0x7c] = 0x46,        // second half of print screen
  [0x7d] = 0x4b         // PAGE UP
};

unsigned char wait4Ack()
{
    unsigned char rxChar;
    unsigned int tstart = time_microsec();
    while (1)
    {
        // wait for answer
        if (getPS2char(&rxChar) == 0)
        {
            if (rxChar == PS2_ACK)
            {
                return 0;   // ok
            }
            else
            {
                ee_printf("[PS/2] unexpected data '%02x' from keyboard\n", rxChar);
                return 1;
            }
        }
        else if ((time_microsec() - tstart) >= RECEIVETIMEOUT)
        {
            ee_printf("[PS/2] no keyboard detected\n");
            return 1;
        }
    }
}

unsigned char initPS2()
{
    unsigned char i;
    // PS/2 Devices use a pull up resistor on both data and clock.
    // Therefore the line can be pulled low with a transistor by the device OR the host.
    // No one ever should produce a short by switching the line direclty to high.
    gpio_select(PS2DATAPIN, GPIO_INPUT);
    gpio_select(PS2CLOCKPIN, GPIO_INPUT);

    gpio_setpull(PS2DATAPIN, GPIO_PULL_UP);
    gpio_setpull(PS2CLOCKPIN, GPIO_PULL_UP);

    gpio_setedgedetect(PS2CLOCKPIN, GPIO_EDGE_DETECT_FALLING);

    irq_attach_handler(49, handlePS2ClockEvent, 0);

    inout.readPos = 0;
    inout.writePos = 0;
    inout.bit_cnt = 0;

    // Disable Scanning
    sendPS2Byte(PS2_DISABLESCANNING);
    if (wait4Ack()) return 1;

    // Get keyboard type
    sendPS2Byte(PS2_IDENTIFYDEV);
    if (wait4Ack()) return 2;
    // wait for 40ms for data
    usleep(2*RECEIVETIMEOUT);
    for (i=0;i<2;i++)
    {
        getPS2char(&inout.keyboardType[i]);
    }

    // Set Scancode 2
    sendPS2Byte(PS2_GETSETSCANCODE);
    if (wait4Ack()) return 3;
    sendPS2Byte(2);        // set scancode 2
    if (wait4Ack()) return 4;

    // Get Scancode
    sendPS2Byte(PS2_GETSETSCANCODE);
    if (wait4Ack()) return 5;
    sendPS2Byte(0);        // get it
    if (wait4Ack()) return 6;
    usleep(RECEIVETIMEOUT); // wait for data
    if (getPS2char(&inout.scanCodeSet) != 0)
    {
        ee_printf("[PS/2] data error\n");
        return 7;
    }
    if (inout.scanCodeSet != 2)
    {
        ee_printf("[PS/2] keyboard seems to not support scancode set 2\n");
        return 8;
    }

    // Re-enable scanning
    sendPS2Byte(PS2_ENABLESCANNING);
    if (wait4Ack()) return 9;

    ee_printf("[PS/2] keyboard type (0x%02x 0x%02x) detected, using scancode set %d\n", inout.keyboardType[0], inout.keyboardType[1], inout.scanCodeSet);
    return 0;
}

unsigned char calcParityBit(unsigned char val)
{
    unsigned char nbrOnes = 0;
    for (unsigned char i=0;i<=7;i++)
    {
        if ((val & 1) != 0) nbrOnes++;
        val = val >> 1;
    }
    return ((nbrOnes & 1) == 0);
}

void sendPS2Byte(unsigned char sendVal)
{
    // save sendval and calc Parity
    inout.sendByte = sendVal;
    inout.sendParity = calcParityBit(sendVal);
    // bring the clock line low for at least 100us
    gpio_select(PS2CLOCKPIN, GPIO_OUTPUT);
    gpio_set(PS2CLOCKPIN, 0);
    usleep(200);
    //first make sure we have no old data in the input buffer
    inout.readPos = inout.writePos;
    inout.bit_cnt = 0;
    // we are sending now, change IRQ handling
    inout.sending = 1;
    // now bring data low
    gpio_select(PS2DATAPIN, GPIO_OUTPUT);
    gpio_set(PS2DATAPIN, 0);
    usleep(200);
    // release the clock
    gpio_select(PS2CLOCKPIN, GPIO_INPUT);
    // now the keyboard generates clocks which get handled by the irq handler
}


void handlePS2ClockEvent(__attribute__((unused)) void* data)
{
    unsigned char data_state;

    inout.bit_cnt++;

    if (inout.sending == 0)
    // receiving from keyboard
    {
        // first bit is startbit (low)      -> 1
        // after this we get 8 data bits    -> 2..9
        // one parity bit (even 1s = 1)     -> 10
        // stop bit (high)                  -> 11
        if (inout.bit_cnt >= 11)
        {
            inout.bit_cnt = 0;
            inout.writePos++;
            inout.writePos %= INPUTBUFFSIZE;
        }
        else if (inout.bit_cnt == 10)
        {
            // Parity check, we don't do this for the moment
        }
        else if (inout.bit_cnt > 1)
        {
            // Data
            data_state = gpio_get(PS2DATAPIN);
            inout.fromKeyboard[inout.writePos] |= (data_state << (inout.bit_cnt - 2));
        }
        else
            inout.fromKeyboard[inout.writePos] = 0;     // init bufferpos
    }
    else
    // sending to keyboard
    {
        // sending to the keyboard
        if (inout.bit_cnt <= 8)
        {
            // data bits
            if (inout.sendByte & 1)
            {
                gpio_select(PS2DATAPIN, GPIO_INPUT);
            }
            else
            {
                gpio_select(PS2DATAPIN, GPIO_OUTPUT);
                gpio_set(PS2DATAPIN, 0);
            }
            inout.sendByte = inout.sendByte >> 1;
        }
        else if (inout.bit_cnt == 9)
        {
            // parity
            if (inout.sendParity)
            {
                gpio_select(PS2DATAPIN, GPIO_INPUT);
            }
            else
            {
                gpio_select(PS2DATAPIN, GPIO_OUTPUT);
                gpio_set(PS2DATAPIN, 0);
            }
        }
        else if (inout.bit_cnt == 10)
        {
            // release datapin
            gpio_select(PS2DATAPIN, GPIO_INPUT);
        }
        else
        {
            // Ack
            inout.sending = 0;
            inout.bit_cnt = 0;
        }
    }

    gpio_clear_irq(PS2CLOCKPIN);
}

unsigned char getPS2char(unsigned char *fromKbd)
{
    if (inout.readPos != inout.writePos)
    {
        //ee_printf("%02x ", inout.fromKeyboard[inout.readPos]);
        *fromKbd = inout.fromKeyboard[inout.readPos];
        inout.readPos++;
        inout.readPos %= INPUTBUFFSIZE;
        return 0;
    }
    return 1;
}

void PS2KeyboardHandler()
{
    unsigned char fromKbd;
    unsigned char usbKey = 0;

    if (getPS2char(&fromKbd) == 0)      // got a char
    {
        // got a char
        switch (ps2ToUsbState)
        {
            case ps2_idle:
                if (fromKbd == 0xe1) ps2ToUsbState = ps2_E1;
                else if (fromKbd == 0xe0) ps2ToUsbState = ps2_E0;
                else if (fromKbd == 0xf0) ps2released = 1;
                else if (ps2released == 1)
                {
                    ps2released = 0;
                    if (fromKbd == 0x11) keyModifiers &= ~ALT;
                    else if (fromKbd == 0x12) keyModifiers &= ~LSHIFT;
                    else if (fromKbd == 0x14) keyModifiers &= ~LCTRL;
                    else if (fromKbd == 0x59) keyModifiers &= ~RSHIFT;
                }
                else
                {
                    // key pressed -> translate
                    if (fromKbd <= 0x83)
                    {
                        if (fromKbd == 0x11) keyModifiers |= ALT;
                        else if (fromKbd == 0x12) keyModifiers |= LSHIFT;
                        else if (fromKbd == 0x14) keyModifiers |= LCTRL;
                        else if (fromKbd == 0x59) keyModifiers |= RSHIFT;

                        usbKey = ps2_set2_to_usb_map[fromKbd];
                    }
                }
                break;

            case ps2_E1:
                // only pause key
                ps2E1CntChars++;
                if ((fromKbd == 0x77) && (ps2E1CntChars == 7))
                {
                    ps2E1CntChars = 0;
                    ps2ToUsbState = ps2_idle;
                    usbKey = 0x48;    // PAUSE key
                }
                else if (ps2E1CntChars >= 7)
                {
                    // data error
                    ps2E1CntChars = 0;
                    ps2ToUsbState = ps2_idle;
                }
                break;

            case ps2_E0:
                if (fromKbd == 0xf0) ps2E0released = 1;
                else if (ps2E0released == 1)
                {
                    if (fromKbd == 0x11) keyModifiers &= ~ALTGR;
                    else if (fromKbd == 0x14) keyModifiers &= ~RCTRL;
                    else if (fromKbd == 0x1f) keyModifiers &= ~LWIN;
                    else if (fromKbd == 0x27) keyModifiers &= ~RWIN;

                    ps2E0released = 0;
                    ps2ToUsbState = ps2_idle;
                }
                else
                {
                    // E0 key pressed -> translate
                    ps2ToUsbState = ps2_idle;
                    if (fromKbd <= 0x7d)
                    {
                        if (fromKbd == 0x11) keyModifiers |= ALTGR;
                        else if (fromKbd == 0x14) keyModifiers |= RCTRL;
                        else if (fromKbd == 0x1f) keyModifiers |= LWIN;
                        else if (fromKbd == 0x27) keyModifiers |= RWIN;

                        usbKey = ps2_set2_e0_to_usb_map[fromKbd];
                    }
                }
                break;
        }
    }
    // Call Key handler
    KeyEvent(usbKey, keyModifiers);
}

void setPS2Leds(unsigned char scroll, unsigned char num, unsigned char caps)
{
    unsigned char combinedState = (scroll << 0) | (num << 1) | (caps << 2);
    sendPS2Byte(PS2_SETLEDS);
    if (wait4Ack()) return;
    sendPS2Byte(combinedState);
    if (wait4Ack()) return;
}


