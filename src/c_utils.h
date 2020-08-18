#ifndef C_UTILS_H__
#define C_UTILS_H__ 

#include <stdint.h>
#include <stddef.h>

void *pigfx_memset (void *pBuffer, int nValue, size_t nLength);
void *qmemcpy(void *dest, void *src, size_t n);
void veryfastmemcpy(void *dest, void* src, unsigned int n);
void *pigfx_memcpy (void *pDest, const void *pSrc, size_t nLength);
char *pigfx_strcpy (char *pDest, const char *pSrc);
size_t pigfx_strlen (const char *pString);
int pigfx_strcmp (const char *pString1, const char *pString2);
int isspace(int c);
char *pigfx_strncpy (char *pDest, const char *pSrc, size_t nMaxLen);
char *strchr(const char *p, int ch);
int32_t atoi(const char *p);

#define memset(a,b,c) pigfx_memset(a,b,c)
#define memcpy(a,b,c) pigfx_memcpy(a,b,c)

#endif
