#include "timer.h"


typedef struct {
    _TimerHandler* handler;
    void* pParam;
    void* pContext;
    unsigned int microsec_interval;
    unsigned int last_tick;

} TimerUnit;


#define N_TIMERS 20
static TimerUnit timers[ N_TIMERS ];

void timers_init()
{
    int i;
    for( i=0; i<N_TIMERS; ++i )
    {
        timers[i].handler = 0;
    }
}

unsigned attach_timer_handler( unsigned hz, _TimerHandler* handler, void *pParam, void* pContext )
{
    // The value hz is really in unit hz. So 1 hz is 1 second delay.
    unsigned hnd;
    if (hz == 0) return 0;
    for( hnd=0; hnd<N_TIMERS; ++hnd )
    {
        if( timers[hnd].handler == 0 )
        {
            timers[hnd].handler = handler;
            timers[hnd].pParam = pParam;
            timers[hnd].pContext = pContext;
            timers[hnd].microsec_interval = 1000000/hz;
            timers[hnd].last_tick = time_microsec();
            return hnd;
        }
    }

    return N_TIMERS+1;

}


void timer_poll()
{
    unsigned hnd;
    unsigned int tnow = time_microsec();

    for( hnd=0; hnd<N_TIMERS; ++hnd )
    {
        if( timers[hnd].handler != 0 && 
            (tnow-timers[hnd].last_tick) > timers[hnd].microsec_interval )
        {
            _TimerHandler* handler = timers[hnd].handler;
            timers[hnd].handler = 0;
            handler( hnd, timers[hnd].pParam, timers[hnd].pContext );
        }
    }
}
