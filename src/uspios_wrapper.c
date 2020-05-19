#include "../uspi/include/uspios.h"
#include "nmalloc.h"
#include "timer.h"
#include "ee_printf.h"
#include "uart.h"
#include "irq.h"
#include "prop.h"
#include "mbox.h"


void *malloc (unsigned nSize)		// result must be 4-byte aligned
{
    return nmalloc_malloc( nSize );
}


void free (void *pBlock)
{
    nmalloc_free( &pBlock );
}


void MsDelay (unsigned nMilliSeconds)
{
    usleep( nMilliSeconds * 1000 );
}


void usDelay (unsigned nMicroSeconds)
{
    usleep( nMicroSeconds );
}


unsigned StartKernelTimer (unsigned nHzDelay,	// in HZ units (see "system configuration" above)
			   TKernelTimerHandler *pHandler,
			   void *pParam, void *pContext)	// handed over to the timer handler
{
    if (nHzDelay == 0) nHzDelay = 1;        // uspi sometimes sends value 0, this means within the next 10ms.
    nHzDelay = nHzDelay * HZ;       // HZ = 100
    return attach_timer_handler( nHzDelay, pHandler, pParam, pContext );
}


void CancelKernelTimer (__attribute__((unused)) unsigned hTimer)
{
    // NOT IMPLEMENTED
    //ee_printf("* CancelKernelTimer *\n");
}


void ConnectInterrupt (unsigned nIRQ, TInterruptHandler *pHandler, void *pParam)
{
    //ee_printf("* ConnectInterrupt * IRQ: %d \n", nIRQ);
    irq_attach_handler( nIRQ, pHandler, pParam );
}


int SetPowerStateOn (unsigned nDeviceId)	// "set power state" to "on", wait until completed
{
    // Set PowerOn State
    typedef struct
    {
        mbox_msgheader_t header;
        mbox_tagheader_t tag;

        union
        {
            struct
            {
                uint32_t deviceId;
                uint32_t state;
            }
            request;
            struct
            {
                uint32_t deviceId;
                uint32_t state;
            }
            response;
        }
        value;

        mbox_msgfooter_t footer;
    }
    message_t;

    message_t msg __attribute__((aligned(16)));

    msg.header.size = sizeof(msg);
    msg.header.code = 0;
    msg.tag.id = MAILBOX_TAG_SET_POWER_STATE;
    msg.tag.size = sizeof(msg.value);
    msg.tag.code = 0;
    msg.value.request.deviceId = nDeviceId;
    msg.value.request.state = 1; // Bit 0: 0=off, 1=on; Bit 1: 0=do not wait, 1=wait; Bits 2-31: reserved for future use (set to 0)
    msg.footer.end = 0;

    if (mbox_send(&msg) != 0) {
        return 0;   // Error
    }
    
    usleep(500000); // Wait some more for wireless keyboards startup time
    return 1;
}


int GetMACAddress (unsigned char Buffer[6])	// "get board MAC address"
{
    //ee_printf("* GetMacAddress *\n");

    if (prop_macaddr(Buffer) != 1)
        return 0;   // error

    return 1;
}


void uspi_assertion_failed (const char *pExpr, const char *pFile, unsigned nLine)
{
    ee_printf("ASSERTION FAILED: %s, in %s (Line %d)\n", pExpr, pFile, nLine);

    while(1)
        usleep(1000000);
}


void DebugHexdump (const void *pBuffer, unsigned nBufLen, const char *pSource /* = 0 */)
{
    ee_printf("Memory dump of %s:\n", pSource );
    uart_dump_mem( (unsigned char*)pBuffer, (unsigned char*)( pBuffer ) + nBufLen );
}
