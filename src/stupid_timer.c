//
// stupid_timer.c
// Software timers
//
// PiGFX is a bare metal kernel for the Raspberry Pi
// that implements a basic ANSI terminal emulator with
// the additional support of some primitive graphics functions.
// Copyright (C) 2014-2020 Filippo Bergamasco, Christian Lehner

#include "peri.h"
#include "timer.h"
#include "utils.h"


typedef struct {
    _TimerHandler* handler;
    void* pParam;
    void* pContext;
    unsigned int microsec_interval;
    unsigned int last_tick;

} TimerUnit;


#define N_TIMERS 20
static TimerUnit timers[ N_TIMERS+1 ];
unsigned int actTicks = 0;

void timers_init()
{
    int i;
    for( i=0; i<=N_TIMERS; ++i )
    {
        timers[i].handler = 0;
    }
    actTicks = time_microsec();
}

unsigned int time_microsec()
{
    return R32(TIMER_CLO); // System Timer Lower 32 bits
}

//  busy-wait a fixed amount of time in us.
void usleep(unsigned int us)
{
    unsigned int tact;
    unsigned int tstart = time_microsec();
    do
    {
        tact = time_microsec();
    } while (tact-tstart < us);
}

unsigned attach_timer_handler( unsigned hz, _TimerHandler* handler, void *pParam, void* pContext )
{
    // The value hz is really in unit hz. So 1 hz is 1 second delay.
    if (hz == 0) return 0;
    return attach_timer_handler_usec(1000000/hz, handler, pParam, pContext);
}

extern unsigned attach_timer_handler_usec( unsigned usec, _TimerHandler* handler, void *pParam, void* pContext )
{
    unsigned hnd;
    for( hnd=1; hnd<=N_TIMERS; ++hnd )
    {
        if( timers[hnd].handler == 0 )
        {
            timers[hnd].handler = handler;
            timers[hnd].pParam = pParam;
            timers[hnd].pContext = pContext;
            timers[hnd].microsec_interval = usec;
            timers[hnd].last_tick = actTicks;
            return hnd;
        }
    }
    return 0;
}

void remove_timer(unsigned hnd)
{
    if (hnd <=N_TIMERS)
        timers[hnd].handler = 0;
}

void timer_poll()
{
    unsigned hnd;
    actTicks = time_microsec();

    for( hnd=1; hnd<=N_TIMERS; ++hnd )
    {
        if( timers[hnd].handler != 0 &&
            (actTicks-timers[hnd].last_tick) > timers[hnd].microsec_interval )
        {
            _TimerHandler* handler = timers[hnd].handler;
            handler( hnd, timers[hnd].pParam, timers[hnd].pContext );
            timers[hnd].handler = 0;
        }
    }
}

struct timer_wait register_timer(useconds_t usec)
{
	struct timer_wait tw;
	tw.rollover = 0;
	tw.trigger_value = 0;

	if(usec < 0)
	{
		return tw;
	}
	uint32_t cur_timer = R32(TIMER_CLO);
	uint32_t trig = cur_timer + (uint32_t)usec;

	if(cur_timer == 0)
		trig = 0;

	tw.trigger_value = trig;
	if(trig > cur_timer)
		tw.rollover = 0;
	else
		tw.rollover = 1;
	return tw;
}

int compare_timer(struct timer_wait tw)
{
	uint32_t cur_timer = R32(TIMER_CLO);

	if(tw.trigger_value == 0)
		return 1;

	if(cur_timer < tw.trigger_value)
	{
		if(tw.rollover)
			tw.rollover = 0;
	}
	else if(!tw.rollover)
		return 1;

	return 0;
}
