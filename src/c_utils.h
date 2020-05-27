#ifndef C_UTILS_H__
#define C_UTILS_H__ 

typedef unsigned long	size_t;

void *pigfx_memset (void *pBuffer, int nValue, size_t nLength);
void *pigfx_memcpy (void *pDest, const void *pSrc, size_t nLength);

#endif
