#include "timer.h"

void start(Timer *t)
{
    t->started = 1;
    t->start_ticks = SDL_GetTicks();
}

void stop(Timer *t)
{
    t->started = 0;
    t->start_ticks = 0;
}

Uint32 get_ticks(Timer *t)
{
    Uint32 times = 0;
    if (t->started)
        times = SDL_GetTicks() - t->start_ticks;

    return times;
}
