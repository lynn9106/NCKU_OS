#include "../include/all_include.h"

int TaskCreate(char *job_name, char *p_function, int priority)
{      
    th_num++;

    thread_tptr new_th = create_thread(job_name, th_num, priority);

    allThreads[th_num-1] = new_th;


    if(strcmp(p_function, "task1") == 0)
        CreateContext(&(new_th->th_ctx), &dispatch_context, &task1);
    else if(strcmp(p_function, "task2") == 0)
        CreateContext(&(new_th->th_ctx), &dispatch_context, &task2);
    else if(strcmp(p_function, "task3") == 0)
        CreateContext(&(new_th->th_ctx), &dispatch_context, &task3);
    else if(strcmp(p_function, "task4") == 0)
        CreateContext(&(new_th->th_ctx), &dispatch_context, &task4);
    else if(strcmp(p_function, "task5") == 0)
        CreateContext(&(new_th->th_ctx), &dispatch_context, &task5);
    else if(strcmp(p_function, "task6") == 0)
        CreateContext(&(new_th->th_ctx), &dispatch_context, &task6);
    else if(strcmp(p_function, "task7") == 0)
        CreateContext(&(new_th->th_ctx), &dispatch_context, &task7);
    else if(strcmp(p_function, "task8") == 0)
        CreateContext(&(new_th->th_ctx), &dispatch_context, &task8);
    else if(strcmp(p_function, "task9") == 0)
        CreateContext(&(new_th->th_ctx), &dispatch_context, &task9);
    else if(strcmp(p_function, "test_exit") == 0)
        CreateContext(&(new_th->th_ctx), &dispatch_context, &test_exit);
    else if(strcmp(p_function, "test_sleep") == 0)
        CreateContext(&(new_th->th_ctx), &dispatch_context, &test_sleep);
    else if(strcmp(p_function, "test_resource1") == 0)
        CreateContext(&(new_th->th_ctx), &dispatch_context, &test_resource1);
    else if(strcmp(p_function, "test_resource2") == 0)
        CreateContext(&(new_th->th_ctx), &dispatch_context, &test_resource2);
    else if(strcmp(p_function, "idle") == 0)
        CreateContext(&(new_th->th_ctx), &dispatch_context, &idle);
    else
    {
        free(new_th);
        return -1;
    }
 
    enq(&new_th, &ready_head);
    printf("Task %s is ready.\n",new_th->th_name);

    return new_th->th_id;
}


void CreateContext(ucontext_t *context, ucontext_t *next_context, void *func)
{
    getcontext(context);
    context->uc_stack.ss_sp = malloc(STACK_SIZE);
    context->uc_stack.ss_size = STACK_SIZE;
    context->uc_stack.ss_flags = 0;
    context->uc_link = next_context;
    makecontext(context,(void (*)(void))func,0);
}

void Dispatcher()
{
    if(ready_head!=NULL){
    run = deq(&ready_head);
    run->th_state = RUNNING;
    printf("Task %s is running.\n",run->th_name); 
    setcontext(&(run->th_ctx));
    }
    else{
        run=NULL;
        if(wait_head==NULL){
        printf("Simulatiom over.\n");
        fflush(stdout);
        simuOver=1;
        setcontext(&startthread);
        }
        else{
            printf("CPU idle.\n");
            CPUidle=1;
            idle();
        }
    }

}


void stopping(int sig){
    printf("\n");
    if(run!=NULL)   {swapcontext(&(run->th_ctx),&mainthread);} 
    else    {swapcontext(&dispatch_context,&mainthread);}
}

void TimeHandler(int sig){
    thread_tptr temp_th = ready_head;
    thread_tptr ex_th = NULL;
    while(temp_th != NULL)                  //task in ready Q
        {
            temp_th->th_waittime += 1;
            temp_th->th_turnaroundtime += 1;
            temp_th = temp_th->th_next;
        }

    temp_th = wait_head;
    ex_th = NULL;
        while(temp_th != NULL)              //task in waiting Q
        {
            temp_th->th_turnaroundtime += 1;

            if(temp_th->th_waitforRes){     //in waiting Q because of waiting resource, check if it can get the resources
                int allresOK=1;
                int counting=0;
                while(counting<8){
                    if(temp_th->th_resource[counting] == 2){
                        if(resource_arr[counting] == 0){
                        allresOK=0;
                        break;
                        }
                    }
                        counting++;
                }

                if(allresOK){
                thread_tptr target = temp_th;
                thread_tptr target_ex = ex_th;
                target->th_waitforRes =0;
                    if(target == wait_head){
                        wait_head = wait_head->th_next;
                    }
                    else{
                        target_ex->th_next = target->th_next;
                        temp_th = target_ex;
                    }
                    target->th_state=READY;
                    target->th_next=NULL;
                    enq(&target, &ready_head);
                }
            }
            else if(temp_th->th_needtowait != 0)    //in waiting Q because of sleeping
            {
                thread_tptr target = temp_th;
                thread_tptr target_ex = ex_th;
                target->th_already_wait++;
                if(target->th_already_wait >= target->th_needtowait){           //check if it should wake up
                    target->th_already_wait =0;
                    target->th_needtowait =0;

                    if(target == wait_head){
                        wait_head = wait_head->th_next;
                    }
                    else{
                        target_ex->th_next = target->th_next;
                        temp_th = target_ex;
                    }
                    target->th_state=READY;
                    target->th_next=NULL;
                    enq(&target, &ready_head);
                }
            }
            ex_th = temp_th;
            temp_th = temp_th->th_next;
        }

    if(run!=NULL){                  //running task
        run->th_runtime+=1;
        run->th_turnaroundtime+=1;
        if(algorithm==1)    //RR
        {
            run->RR_time+=1;
            if(run->RR_time!= 0 && run->RR_time%3 == 0){
                run->th_state = READY;
                run->RR_time=0;
                run->th_next=NULL;
                enq(&run, &ready_head);
                swapcontext(&(run->th_ctx),&dispatch_context);
            }
        }
        else if(algorithm==2)   //PP
        {
            int reQ=0;
            thread_tptr t_th = ready_head;
            while(t_th != NULL)                  //task in ready Q
            {
                if(t_th->th_priority < run->th_priority){
                    reQ =1;
                }
                t_th = t_th->th_next;
            }
            if(reQ){                //preemptive
                run->th_state = READY;
                run->th_next=NULL;
                enq(&run, &ready_head);
                swapcontext(&(run->th_ctx),&dispatch_context);
            }

        }
    }



    if(CPUidle){
            if(ready_head!=NULL && run==NULL){
            CPUidle=0;
            setcontext(&dispatch_context);
            }
    }
}






