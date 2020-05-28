/* 2020 Christian Lehner
 * 
 * 
 * */
#include <stdint.h>
#include "c_utils.h"
#include "utils.h"

void *pigfx_memset (void *pBuffer, int nValue, size_t nLength)
{
	char *p = (char *) pBuffer;

	while (nLength--)
	{
		*p++ = (char) nValue;
	}

	return pBuffer;
}

// A faster memcpy if 16-byte alignment is assured
void *qmemcpy(void *dest, void *src, size_t n)
{
	// Can only use quick_memcpy if dest, src and n are multiples
	//  of 16
	if((((uintptr_t)dest & 0xf) == 0) && (((uintptr_t)src & 0xf) == 0) &&
		((n & 0xf) == 0))
		return quick_memcpy(dest, src, n);
	else
		return pigfx_memcpy(dest, src, n);
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

char *pigfx_strcpy (char *pDest, const char *pSrc)
{
	char *p = pDest;

	while (*pSrc)
	{
		*p++ = *pSrc++;
	}

	*p = '\0';

	return pDest;
}

size_t pigfx_strlen (const char *pString)
{
	size_t nResult = 0;

	while (*pString++)
	{
		nResult++;
	}

	return nResult;
}

int pigfx_strcmp (const char *pString1, const char *pString2)
{
	while (   *pString1 != '\0'
	       && *pString2 != '\0')
	{
		if (*pString1 > *pString2)
		{
			return 1;
		}
		else if (*pString1 < *pString2)
		{
			return -1;
		}

		pString1++;
		pString2++;
	}

	if (*pString1 > *pString2)
	{
		return 1;
	}
	else if (*pString1 < *pString2)
	{
		return -1;
	}

	return 0;
}

