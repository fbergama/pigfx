#include "dma.h"
#include "utils.h"
#include "mbox.h"
#include "console.h"


#define DMA_CS_OFFSET        0x00
#define DMA_CONBLK_AD_OFFSET 0x01
// https://www.raspberrypi.org/forums/viewtopic.php?f=72&t=10276


typedef struct _DMA_Ctrl_Block
{
    unsigned int TI;                    // Transfer information
    unsigned int SOURCE_AD;             // source address 
    unsigned int DEST_AD;               // destination address 
    unsigned int TXFR_LEN;              // transfer length 
    unsigned int STRIDE;                // 2D mode stride
    struct _DMA_Ctrl_Block* NEXTCONBK;  // Next control block address 
    unsigned int reserved1;
    unsigned int reserved2;

} DMA_Control_Block;


DMA_Control_Block __attribute__((aligned(0x100))) ctr_blocks[16];
unsigned int curr_blk;

unsigned int channel;

void dma_init()
{
    curr_blk = 0;
    channel = 0;
    // Enable DMA on used channel
    unsigned int actEnable = R32(DMA_ENABLE);
    actEnable |= (1 << channel);
    W32(DMA_ENABLE, actEnable);
}


int dma_enqueue_operation( void* src, void* dst, unsigned int len, unsigned int stride, unsigned int TRANSFER_INFO )
{
    // Y length in 2D mode is limited to 16384 as the 2 top bits are reserved
    if( curr_blk == 16 )
        return 0;

    DMA_Control_Block* blk = &( ctr_blocks[ curr_blk ]);
    blk->TI = TRANSFER_INFO;
    blk->SOURCE_AD = mem_arm2vc((unsigned int)src);
    blk->DEST_AD = mem_arm2vc((unsigned int)dst);
    blk->TXFR_LEN = len;
    blk->STRIDE = stride;
    blk->NEXTCONBK = 0;
    blk->reserved1 = 0;
    blk->reserved2 = 0;

    if( curr_blk > 0 )
    {
        // Enqueue 
        ctr_blocks[ curr_blk -1 ].NEXTCONBK = blk;
    }

    ++curr_blk;
    return curr_blk;
}  


void dma_execute_queue()
{
    // Set the first control block
    *( (volatile unsigned int*)DMA_BASE + (channel << 6) + DMA_CONBLK_AD_OFFSET ) = mem_arm2vc((unsigned int)ctr_blocks);

    // Start the operation
    *( (volatile unsigned int*)DMA_BASE + (channel << 6) + DMA_CS_OFFSET  ) = 7;

    // reset the queue
    curr_blk=0;
    
    // wait for DMA to finish
    while( dma_running() )
    {
        ;// Busy wait for DMA
    }
}


int dma_running()
{
    return *( (volatile unsigned int*)DMA_BASE + (channel << 6) + DMA_CS_OFFSET  ) &  0x1;
}


void dma_memcpy_32( void* src, void *dst, unsigned int size )
{
    dma_enqueue_operation( src, dst, size, 0, DMA_TI_SRC_INC | DMA_TI_DEST_INC ); 
    dma_execute_queue();
}


