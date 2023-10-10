#include "../include/all_include.h"

void task_sleep(int ms)
{
        printf("Task %s goes to sleep.\n",run->th_name);
        run->th_state = WAITING;
        run->th_needtowait = ms;
        run->RR_time =0;
        enq(&run, &wait_head);//change to waiting state
        swapcontext(&(run->th_ctx), &dispatch_context);
}

void task_exit()
{
        printf("Task %s has terminated.\n",run->th_name);
        run->th_state = TERMINATED;
        enq(&run, &terminate_head);//change to terminate state
        run=NULL;
        setcontext(&dispatch_context);
}
