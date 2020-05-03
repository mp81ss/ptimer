#include "ptimer.h"

#ifdef _WIN32
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#else
#   include <unistd.h>
#endif

#include <stdlib.h>
#include <time.h>
#include <semaphore.h>
#include <errno.h>

struct ptimer {
    sem_t               semaphore;
    ptimer_callback_t   callback;
    void*               callback_argument;
    ptimer_type         type;
    unsigned int        interval;
    int                 is_ready;
};

int ptimer_create(ptimer_ptr_t* p_timer_address,
                  ptimer_type type,
                  unsigned int interval,
                  ptimer_callback_t callback,
                  void* callback_argument)
{
    int ok = 0;
    struct ptimer* const p = malloc(sizeof(struct ptimer));

    if (p != NULL) {

        if (sem_init(&p->semaphore, 0, 0U) == 0) {

            p->callback          = callback;
            p->callback_argument = callback_argument;
            p->type              = type;
            p->interval          = interval;
            p->is_ready          = -1;

            *p_timer_address = p;

            ok--;
        }
        else {
            free(p);
        }
    }

    return ok;
}

void ptimer_start(ptimer_ptr_t p_timer)
{
    struct ptimer* const p = (struct ptimer*)p_timer;
    const int periodic = p->type == PTIMER_PERIODIC;
    struct timespec ts;

    do {
        if (clock_gettime(CLOCK_REALTIME, &ts) == 0) {
            const unsigned int interval = p->interval;
            ts.tv_sec  += (time_t)(interval / 1000U);
            ts.tv_nsec += (long)((interval % 1000U) * 1000000L);
            if (ts.tv_nsec >= 1000000000L) {
                ts.tv_sec++;
                ts.tv_nsec -= 1000000000L;
            }
            while (sem_timedwait(&p->semaphore, &ts) == -1 && errno == EINTR) {
                continue;
            }
        }
        p->callback(p->callback_argument);
    } while (periodic != 0 && p->is_ready != 0);
}

void ptimer_stop(ptimer_ptr_t p_timer)
{
    struct ptimer* const p = (struct ptimer*)p_timer;
    p->is_ready = 0;
    sem_post(&p->semaphore);
}

void ptimer_destroy(ptimer_ptr_t p_timer)
{
    struct ptimer* const p = (struct ptimer* const)p_timer;
    sem_destroy(&p->semaphore);
    free(p_timer);
}

void ptimer_set_timeout(ptimer_ptr_t p_timer, unsigned int timeout)
{
    struct ptimer* const p = (struct ptimer*)p_timer;
    p->interval = timeout;
}

void ptimer_set_callback(ptimer_ptr_t p_timer, ptimer_callback_t new_callback)
{
    struct ptimer* const p = (struct ptimer*)p_timer;
    p->callback = new_callback;
}

void ptimer_set_callback_argument(ptimer_ptr_t p_timer, void* callback_argument)
{
    struct ptimer* const p = (struct ptimer*)p_timer;
    p->callback_argument = callback_argument;
}

void ptimer_sleep(unsigned int interval)
{
#ifdef _WIN32
    Sleep((DWORD)interval);
#else
    usleep((useconds_t)(interval * 1000U));
#endif
}
