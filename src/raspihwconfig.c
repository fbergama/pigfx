#include "raspihwconfig.h"
#include "postman.h"
#include "utils.h"
#include "timer.h"


RHW_RETURN_TYPE rhw_get_mac_address( unsigned char* pOutAddr )
{
    volatile unsigned int pBuffData[256] __attribute__((aligned (16)));
    unsigned int off;
    unsigned int respmsg;
    unsigned char* aux;

    off=1;
    pBuffData[off++] = 0;           // Request 
    pBuffData[off++] = 0x00010003;  // Tag: get board mac address 
    pBuffData[off++] = 6;           // response buffer size in bytes
    pBuffData[off++] = 0;           // request size 
    pBuffData[off++] = 0;           // response buffer
    pBuffData[off++] = 0;           // response buffer
    pBuffData[off++] = 0;           // end tag

    pBuffData[0]=off*4; // Total message size

    if( POSTMAN_SUCCESS != postman_send( 8, mem_v2p((unsigned int)pBuffData) ) )
        return RHW_POSTMAN_FAIL;

    if( POSTMAN_SUCCESS != postman_recv( 8, &respmsg) )
        return RHW_POSTMAN_FAIL;

    if( pBuffData[1]!=0x80000000 )
    {
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
    volatile unsigned int pBuffData[256] __attribute__((aligned (16)));
    unsigned int off;
    unsigned int respmsg;
    unsigned int statebit = (state==RHW_POWER_ON?(1<<0):(0<<0)) | (1<<1);

    off=1;
    pBuffData[off++] = 0;           // Request 
    pBuffData[off++] = 0x00028001;  // Tag: set power state 
    pBuffData[off++] = 8;           // response buffer size in bytes
    pBuffData[off++] = 8;           // request size 
    pBuffData[off++] = dev;         // request/response buffer
    pBuffData[off++] = statebit;    // request/response buffer
    pBuffData[off++] = 0;           // end tag

    pBuffData[0]=off*4; // Total message size

    if( POSTMAN_SUCCESS != postman_send( 8, mem_v2p((unsigned int)pBuffData) ) )
        return RHW_POSTMAN_FAIL;

    unsigned int n_retries = 10;
    unsigned char success = 0;

    while( n_retries-- )
    {
        if( POSTMAN_SUCCESS == postman_recv( 8, &respmsg) )
        {
            success = 1;
            break;
        }
        usleep( 500000 );
    }

    if( !success )
        return RHW_POSTMAN_FAIL;

    if( pBuffData[1]!=0x80000000 )
    {
        return RHW_ERROR;
    }

    /*
    aux = (unsigned char*)&(pBuffData[5]);

    for( off=0; off<6; ++off )
    {
        *pOutAddr++ = *aux++;
    }*/

    return RHW_SUCCESS;

}
