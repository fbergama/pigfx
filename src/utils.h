#ifndef _PIGFX_UTILS_H_
#define _PIGFX_UTILS_H_

extern void enable_irq();
extern void disable_irq();

extern void W32( unsigned int addr, unsigned int data );
extern unsigned int R32( unsigned int addr );

/**
 * String related
 */
extern unsigned int hex2byte( unsigned char* addr );
extern void byte2hexstr( unsigned char byte, char* outstr );
extern void word2hexstr( unsigned int word, char* outstr );
extern unsigned int strlen( char* str );
extern int strcmp( char*s1, char* s2 );
extern void dummy ( unsigned int );

/**
 * Memory
 */
/*
inline void memcpy( unsigned char* dst, unsigned char* src, unsigned int len )
{
    unsigned char* srcEnd = src + len;
    while( src<srcEnd )
    {
        *dst++ =  *src++;
    }
}
*/

/*
 *   Data memory barrier
 *   No memory access after the DMB can run until all memory accesses before it
 *    have completed
 *    
 */
#if RPI==1
#define dmb() asm volatile ("mcr p15, 0, %0, c7, c10, 5" : : "r" (0) : "memory")
#else
#define dmb() asm volatile ("dmb" ::: "memory")
#endif


/*
 *  Data synchronisation barrier
 *  No instruction after the DSB can run until all instructions before it have
 *  completed
 */
#define dsb() asm volatile \
                ("mcr p15, #0, %[zero], c7, c10, #4" : : [zero] "r" (0) )


/*
 * Clean and invalidate entire cache
 * Flush pending writes to main memory
 * Remove all data in data cache
 */
#define flushcache() asm volatile \
                ("mcr p15, #0, %[zero], c7, c14, #0" : : [zero] "r" (0) )





#endif
