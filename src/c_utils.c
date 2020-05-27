/* 2020 Christian Lehner
 * 
 * 
 * */
#include "c_utils.h"

void *pigfx_memset (void *pBuffer, int nValue, size_t nLength)
{
	char *p = (char *) pBuffer;

	while (nLength--)
	{
		*p++ = (char) nValue;
	}

	return pBuffer;
}

void *pigfx_memcpy (void *pDest, const void *pSrc, size_t nLength)
{
	char *pd = (char *) pDest;
	char *ps = (char *) pSrc;

	while (nLength--)
	{
		*pd++ = *ps++;
	}

	return pDest;
} 
