#include "raspihwconfig.h"
#include "mbox.h"
#include "utils.h"
#include "timer.h"


RHW_RETURN_TYPE rhw_get_mac_address( unsigned char* pOutAddr )
{
    // TODO: This should be done like the other mailbox calls
    unsigned int pBuffData[256] __attribute__((aligned (16)));
    unsigned int off;
    unsigned char* aux;

    off=1;
    pBuffData[off++] = 0;           // Request 
    pBuffData[off++] = MAILBOX_TAG_GET_BOARD_MAC_ADDRESS;  // Tag: get board mac address 
    pBuffData[off++] = 6;           // response buffer size in bytes
    pBuffData[off++] = 0;           // request size 
    pBuffData[off++] = 0;           // response buffer
    pBuffData[off++] = 0;           // response buffer
    pBuffData[off++] = 0;           // end tag

    pBuffData[0]=off*4; // Total message size
    
    if (mbox_send(pBuffData) != 0) {
        return RHW_ERROR;
    }

    aux = (unsigned char*)&(pBuffData[5]);

    for( off=0; off<6; ++off )
    {
        *pOutAddr++ = *aux++;
    }

    return RHW_SUCCESS;
}


RHW_RETURN_TYPE rhw_set_device_power( RHW_DEVICE dev, RHW_POWER_STATE state )
{
    // TODO: This should be done like the other mailbox calls
    unsigned int pBuffData[256] __attribute__((aligned (16)));
    unsigned int off;
    unsigned int statebit = (state==RHW_POWER_ON?(1<<0):(0<<0)) | (1<<1);

    off=1;
    pBuffData[off++] = 0;           // Request 
    pBuffData[off++] = MAILBOX_TAG_SET_POWER_STATE;  // Tag: set power state 
    pBuffData[off++] = 8;           // response buffer size in bytes
    pBuffData[off++] = 8;           // request size 
    pBuffData[off++] = dev;         // request/response buffer
    pBuffData[off++] = statebit;    // request/response buffer
    pBuffData[off++] = 0;           // end tag

    pBuffData[0]=off*4; // Total message size
    
    if (mbox_send(pBuffData) != 0) {
        return RHW_ERROR;
    }

    return RHW_SUCCESS;
}
