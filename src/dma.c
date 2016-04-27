#include "dma.h"
#include "utils.h"


#define DMA_BASE 0x20007000
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

void dma_init()
{
    curr_blk = 0;
}


int dma_enqueue_operation( unsigned int* src, unsigned int *dst, unsigned int len, unsigned int stride, unsigned int TRANSFER_INFO )
{
    if( curr_blk == 16 )
        return 0;

    DMA_Control_Block* blk = (DMA_Control_Block*)mem_2uncached( &( ctr_blocks[ curr_blk ]) );
    blk->TI = TRANSFER_INFO;
    blk->SOURCE_AD = (unsigned int)src;
    blk->DEST_AD = (unsigned int)dst;
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

    // Enable DMA on channel 0
    *( (volatile unsigned int*)( DMA_BASE + 0xFF0) ) = 1;

    // Set the first control block
    unsigned int channel = 0;
    *( (volatile unsigned int*)DMA_BASE + (channel << 6) + DMA_CONBLK_AD_OFFSET ) = mem_2uncached( &(ctr_blocks[0]) );

    // Start the operation
    *( (volatile unsigned int*)DMA_BASE + (channel << 6) + DMA_CS_OFFSET  ) = 7;

    // reset the queue
    curr_blk=0;
}


int dma_running()
{
    unsigned int channel = 0;
    return *( (volatile unsigned int*)DMA_BASE + (channel << 6) + DMA_CS_OFFSET  ) &  0x1;
}


void dma_memcpy_32( unsigned int* src, unsigned int *dst, unsigned int size )
{
    dma_enqueue_operation( src, dst, size, 0, DMA_TI_SRC_INC | DMA_TI_DEST_INC ); 
    dma_execute_queue();
}


