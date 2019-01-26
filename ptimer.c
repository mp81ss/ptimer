#include "ptimer.h"

#ifdef _WIN32
#   define WIN32_LEAN_AND_MEAN
#   include <Windows.h>
#else
#   include <unistd.h>
#endif

#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>


typedef enum {
    STATUS_RUNNING,
    STATUS_NOT_RUNNING,
    STATUS_DESTROYED
} timer_status_t;

struct ptimer {
    pthread_t           thread;
    sem_t               semaphore;
    ptimer_callback_t   callback;
    void*               callback_argument;
    ptimer_type         type;
    unsigned int        interval;
    timer_status_t      status;
};


static void* runner(void* p_thread_argument)
{
    struct ptimer* const p = (struct ptimer*)p_thread_argument;

    while (p->status != STATUS_DESTROYED) {

        sem_wait(&p->semaphore);

        while (p->status == STATUS_RUNNING) {

            ptimer_sleep(p->interval);

            if (p->status == STATUS_RUNNING) {

                (*p->callback)(p->callback_argument);

                if (p->type == PTIMER_SINGLE_SHOT) {
                    p->status = STATUS_NOT_RUNNING;
                }
            }
        }
    }

    sem_destroy(&p->semaphore);
    free(p_thread_argument);

    pthread_exit(NULL);

    return NULL;
}

static void set_status(ptimer_ptr_t p_timer, timer_status_t status)
{
    struct ptimer* const p = (struct ptimer* const)p_timer;
    p->status = status;
}

void ptimer_sleep(unsigned int interval)
{
#ifdef _WIN32
    Sleep((DWORD)interval);
#else
    usleep((useconds_t)(interval * 1000));
#endif
}

int ptimer_create(ptimer_ptr_t* p_timer_address,
                  ptimer_type type,
                  unsigned int interval,
                  ptimer_callback_t callback,
                  void* callback_argument)
{
    struct ptimer* const p = malloc(sizeof(struct ptimer));
    int ok = 0;

    if (p != NULL) {

        p->callback          = callback;
        p->callback_argument = callback_argument;
        p->type              = type;
        p->interval          = interval;
        p->status            = STATUS_NOT_RUNNING;

        if (sem_init(&p->semaphore, 0, 0) == 0) {

            if (pthread_create(&p->thread, NULL, &runner, p) == 0) {
                *p_timer_address = p;
                ok = 1;
            }
            else {
                sem_destroy(&p->semaphore);
                *p_timer_address = NULL;
                free(p);
            }
        }
        else {
            *p_timer_address = NULL;
            free(p);
        }
    }

    return ok;
}

void ptimer_start(ptimer_ptr_t p_timer)
{
    struct ptimer* const p = (struct ptimer* const)p_timer;

    if (p->status == STATUS_NOT_RUNNING) {
        set_status(p_timer, STATUS_RUNNING);
        sem_post(&p->semaphore);
    }
}

void ptimer_stop(ptimer_ptr_t p_timer)
{
    set_status(p_timer, STATUS_NOT_RUNNING);
}

void ptimer_set_timeout(ptimer_ptr_t p_timer, unsigned int timeout)
{
    struct ptimer* const p = (struct ptimer* const)p_timer;
    p->interval = timeout;
}

void ptimer_set_callback(ptimer_ptr_t p_timer, ptimer_callback_t new_callback)
{
    struct ptimer* const p = (struct ptimer* const)p_timer;
    p->callback = new_callback;
}

void ptimer_set_callback_argument(ptimer_ptr_t p_timer, void* callback_argument)
{
    struct ptimer* const p = (struct ptimer* const)p_timer;
    p->callback_argument = callback_argument;
}

void ptimer_destroy(ptimer_ptr_t p_timer, ptimer_wait_mode wait_mode)
{
    if (wait_mode == PTIMER_WAIT || wait_mode == PTIMER_NO_WAIT) {
        struct ptimer* const p = (struct ptimer* const)p_timer;

        set_status(p_timer, STATUS_DESTROYED);

        sem_post(&p->semaphore);

        if (wait_mode == PTIMER_WAIT) {
            void* retval;
            pthread_join(p->thread, &retval);
        }
    }
}

int ptimer_is_running(const ptimer_ptr_t p_timer)
{
    const struct ptimer* const p = (const struct ptimer*)p_timer;
    return p->status == STATUS_RUNNING;
}
