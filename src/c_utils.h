#ifndef C_UTILS_H__
#define C_UTILS_H__ 

typedef unsigned long	size_t;

void *pigfx_memset (void *pBuffer, int nValue, size_t nLength);
void *qmemcpy(void *dest, void *src, size_t n);
void *pigfx_memcpy (void *pDest, const void *pSrc, size_t nLength);
char *pigfx_strcpy (char *pDest, const char *pSrc);
size_t pigfx_strlen (const char *pString);
int pigfx_strcmp (const char *pString1, const char *pString2);

#endif
