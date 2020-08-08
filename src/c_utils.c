/* 2020 Christian Lehner
 * 
 * 
 * */
#ifndef C_UTILS_H_
#define C_UTILS_H_

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

void veryfastmemcpy(void *dest, void* src, unsigned int n)
{
    //ee_printf("veryfast\n");
    if ((((unsigned int)dest & 0x3) == 0) && (((unsigned int)src & 0x3) == 0))
    {
        // both 4 byte aligned
        int single = n & 0x3;
        int singpos = n - single;
        int quad = singpos / 4;
        unsigned int* qdest = dest;
        unsigned int* qsrc = src;
        unsigned char* sdest = (unsigned char*)dest+singpos;
        unsigned char* ssrc = (unsigned char*)src+singpos;
        for (int q=0; q<quad; q++)
        {
            qdest[q] = qsrc[q];
        }
        for (int s=0; s<single; s++)
        {
            sdest[s] = ssrc[s];
        }
    }
    else if ((((unsigned int)dest & 0x1) == 0) && (((unsigned int)src & 0x1) == 0))
    {
        // both 2 byte aligned
        int single = n & 0x1;
        int singpos = n - single;
        int dbl = singpos / 2;
        unsigned short int* ddest = dest;
        unsigned short int* dsrc = src;
        unsigned char* sdest = (unsigned char*)dest + singpos;
        unsigned char* ssrc = (unsigned char*)src + singpos;
        for (int d=0; d<dbl; d++)
        {
            ddest[d] = dsrc[d];
        }
        if (single) *sdest = *ssrc;
    }
    else
    {
        pigfx_memcpy(dest, src, n);
    }
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

int isspace(int c)
{
	return (c == '\t' || c == '\n' ||
	    c == '\v' || c == '\f' || c == '\r' || c == ' ' ? 1 : 0);
}

char *pigfx_strncpy (char *pDest, const char *pSrc, size_t nMaxLen)
{
	char *pResult = pDest;

	while (nMaxLen > 0)
	{
		if (*pSrc == '\0')
		{
			break;
		}

		*pDest++ = *pSrc++;
		nMaxLen--;
	}

	if (nMaxLen > 0)
	{
		*pDest = '\0';
	}

	return pResult;
}

char *strchr(const char *p, int ch)
{
	char c;

	c = ch;
	for (;; ++p) {
		if (*p == c)
			return ((char *)p);
		if (*p == '\0')
			return (0);
	}
	/* NOTREACHED */
}

int32_t atoi(const char *p)
/* Copyright (c) 2000-2002 Opsycon AB  (www.opsycon.se) */
{
	int32_t digit, isneg;
	int32_t value;

	isneg = 0;
	value = 0;
	for (; isspace (*p); p++);	/* gobble up leading whitespace */

	/* do I have a sign? */
	if (*p == '-') {
		isneg = 1;
		p++;
	}
	else if (*p == '+')
		p++;

	for (; *p; p++) {
		if (*p >= '0' && *p <= '9')
			digit = *p - '0';
		else
			break;
		value *= 10;
		value += digit;
	}

	if (isneg)
		value = 0 - value;
	return (value);
}

#endif
