#include <stdio.h>
#include "ptimer.h"

static void cb_periodic(void* arg)
{
    static unsigned int counter = 0;

    if (++counter == 3) {
        puts("Goodbye by periodic timer");

        /* Here you can change callback, interval ... */

        ptimer_stop((ptimer_ptr_t)arg); /* ... and even stop the timer! */
    }
    else {
        puts("Hello by periodic timer");
    }
}

static void test_periodic(void)
{
    ptimer_ptr_t periodic;

    if (ptimer_create(&periodic,
                      PTIMER_PERIODIC,
                      PTIMER_SECONDS(1),
                      &cb_periodic,
                      NULL)
        == 0)
    {
        puts("Cannot create periodic timer");
    }

    /* Pass the timer to its callback, cannot be passed at creation time */
    ptimer_set_callback_argument(periodic, periodic);

    ptimer_start(periodic);

    puts("Periodic timer finished");

    ptimer_destroy(periodic);
}

static void cb_single(void* arg)
{
    puts((const char*)arg);
}

static void test_single(void)
{
    ptimer_ptr_t single;
    const char* msg = "Hello from single timer";

    if (ptimer_create(&single,
                      PTIMER_SINGLE_SHOT,
                      PTIMER_SECONDS(2),
                      &cb_single,
                      (void*)msg)
        == 0)
    {
        puts("Cannot create single timer");
    }

    ptimer_start(single);
    ptimer_destroy(single);
}

int main(void)
{
    test_single();
    test_periodic();
    puts("Timers finished, exiting in 2 seconds...");
    ptimer_sleep(PTIMER_SECONDS(2));
    return 0;
}
