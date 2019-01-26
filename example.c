#include <stdio.h>
#include <assert.h>
#include "ptimer.h"


static ptimer_ptr_t p_periodic;
static ptimer_ptr_t timer;
static int done;


static void cb_periodic(void* argument)
{
    printf("timer callback reports: <%s>\n", (char*)argument);
}

static void cb_single(void* argument)
{
    puts("timer single callback");

    (void)argument;
}

static void cb_test2(void* argument)
{
    static int counter = 0;

    printf("cb2 says: <%s>\n", (char*)argument);

    if (++counter == 8) {
        puts("DONE");
        done--;
        assert(ptimer_is_running(timer));
        ptimer_stop(timer);
    }
}

static void cb_test(void* argument)
{
    static int counter = 0;
    printf("cb1 says: %s\n", (char*)argument);

    if (++counter == 5) {
        ptimer_set_callback(timer, &cb_test2);
        ptimer_set_callback_argument(timer, "MSG2");
        ptimer_set_timeout(timer, 500);
    }
}

static void test(void)
{
    if (ptimer_create(&timer, PTIMER_PERIODIC, 1000, &cb_test, "MSG1") != 0) {
        ptimer_start(timer);
        while (!done) ptimer_sleep(100);
        assert(!ptimer_is_running(timer));
        puts("Destorying timer...");
        ptimer_destroy(timer, PTIMER_WAIT);
    }
    else assert(0);
}

int main(void)
{
    const char* msg = "Hello";
    ptimer_ptr_t p_single;

    if (ptimer_create(&p_periodic,
                      PTIMER_PERIODIC,
                      PTIMER_SECONDS(2),
                      &cb_periodic,
                      (void*)msg)
        == 0)
    {
        puts("Cannot create timer");
        return 1;
    }

    if (ptimer_create(&p_single,
                      PTIMER_SINGLE_SHOT,
                      PTIMER_SECONDS(1),
                      &cb_single,
                      NULL)
        == 0)
    {
        puts("Cannot create timer");
        ptimer_destroy(p_periodic, PTIMER_NO_WAIT);
        return 1;
    }

    ptimer_start(p_periodic);

    ptimer_sleep(PTIMER_SECONDS(4));

    ptimer_start(p_single);

    test();

    ptimer_destroy(p_single, PTIMER_WAIT);
    ptimer_destroy(p_periodic, PTIMER_WAIT);

    return 0;
}
