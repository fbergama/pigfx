#ifndef _DMA_H_
#define _DMA_H_


#define DMA_TI_SRC_INC              (1<<8)
#define DMA_TI_SRC_WIDTH_128BIT     (1<<9)
#define DMA_TI_DEST_INC             (1<<4)
#define DMA_TI_DEST_WIDTH_128BIT    (1<<5)
#define DMA_TI_2DMODE               (1<<1)
#define DMA_TI_INTEN                (1<<0)


void dma_init();
int dma_enqueue_operation( unsigned int* src, unsigned int *dst, unsigned int len, unsigned int stride, unsigned int TRANSFER_INFO );   
void dma_execute_queue();
void dma_memcpy_32( unsigned int* src, unsigned int *dst, unsigned int size );

#endif
