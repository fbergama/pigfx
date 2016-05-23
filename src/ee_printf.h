#ifndef _EE_PRINTF_H_
#define _EE_PRINTF_H_


extern void ee_printf(const char *fmt, ...);

extern void LogWrite (const char *pSource,		// short name of module
        	       unsigned	   Severity,		// see above
	       const char *pMessage, ...);	// uses printf format options

#endif
