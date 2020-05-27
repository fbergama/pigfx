#ifndef _PIGFX_UTILS_H_
#define _PIGFX_UTILS_H_

#include <stdint.h>

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
/*extern unsigned int strlen( char* str );
extern int strcmp( char*s1, char* s2 );*/
extern void dummy ( unsigned int );

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


// Support for unaligned data access
static inline void write_word(uint32_t val, uint8_t *buf, int offset)
{
    buf[offset + 0] = val & 0xff;
    buf[offset + 1] = (val >> 8) & 0xff;
    buf[offset + 2] = (val >> 16) & 0xff;
    buf[offset + 3] = (val >> 24) & 0xff;
}

static inline uint32_t read_word(const uint8_t *buf, int offset)
{
	uint32_t b0 = buf[offset + 0] & 0xff;
	uint32_t b1 = buf[offset + 1] & 0xff;
	uint32_t b2 = buf[offset + 2] & 0xff;
	uint32_t b3 = buf[offset + 3] & 0xff;

	return b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
}


// Support for BE to LE conversion
#ifdef __GNUC__
#define byte_swap __builtin_bswap32
#else
static inline uint32_t byte_swap(uint32_t in)
{
	uint32_t b0 = in & 0xff;
    uint32_t b1 = (in >> 8) & 0xff;
    uint32_t b2 = (in >> 16) & 0xff;
    uint32_t b3 = (in >> 24) & 0xff;
    uint32_t ret = (b0 << 24) | (b1 << 16) | (b2 << 8) | b3;
    return ret;
}
#endif // __GNUC__

#endif
