#ifndef _PIGFX_TIMER_H_
#define _PIGFX_TIMER_H_

#include <stdint.h>

typedef int useconds_t;

extern void usleep( unsigned int usec );
extern unsigned int time_microsec();

struct timer_wait
{
	uint32_t trigger_value;
	int rollover;
};

struct timer_wait register_timer(useconds_t usec);
int compare_timer(struct timer_wait tw);

#define TIMEOUT_WAIT(stop_if_true, usec) 		\
do {							\
	struct timer_wait tw = register_timer(usec);	\
	do						\
	{						\
		if(stop_if_true)			\
			break;				\
	} while(!compare_timer(tw));			\
} while(0);

typedef void _TimerHandler(unsigned hTimer, void *pParam, void *pContext);

extern void timers_init();
extern unsigned attach_timer_handler( unsigned hz, _TimerHandler* handler, void *pParam, void* pContext );
extern void timer_poll();


#endif
