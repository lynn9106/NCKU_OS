#include "../include/all_include.h"
#include <string.h>

thread_tptr create_thread(char *job_name, int th_id, int priority)
{
    //change priority into integer
    thread_t *new_th = malloc(sizeof(thread_t));
    new_th->th_name = malloc(15*sizeof(char));
    strcpy(new_th->th_name,job_name);
    new_th->th_id = th_id;      //th_id = th_num(original), th_num = th_num+1
    new_th->th_priority = priority;
    new_th->th_state = READY;
    new_th->th_next = NULL;
    for(int i=0;i<8;i++) new_th->th_resource[i] = 0;
    new_th->th_resCount =0;
    new_th->th_waitforRes =0;

    new_th->th_runtime = 0;
    new_th->th_waittime = 0;
    new_th->th_turnaroundtime = 0;
    new_th->th_already_wait = 0;
    new_th->th_needtowait = 0;

    new_th->RR_time=0;

    return new_th;
}


void enq(thread_tptr *new_th, thread_tptr *head)
{
    thread_tptr temp = (*head);
    thread_tptr temp_ex = NULL;
    
    if(algorithm == 0 || algorithm == 1){   
        if(temp!=NULL){              //FCFS or RR
        while(temp!=NULL){
                temp_ex = temp;
                temp = temp->th_next;
        }
        if(temp == NULL)
            temp_ex ->th_next = (*new_th);
        }
        else
        {
        (*head) = (*new_th);
        (*new_th)->th_next = NULL;
        }
    }
    else if(algorithm == 2){            //PP
        if(temp!=NULL)
        {
            while(temp != NULL)
            {
                if(temp->th_priority < (*new_th)->th_priority)
                {
                    temp_ex = temp;
                    temp = temp->th_next;
                }
                else
                {
                    (*new_th)->th_next = temp;
                    if(temp != (*head))
                        temp_ex->th_next = (*new_th);
                    else
                        (*head) = (*new_th);
                    break;
                }
            }
            if(temp == NULL)
                temp_ex ->th_next = (*new_th);
        }
        else//empty queue
        {
            (*head) = (*new_th);
            (*new_th)->th_next = NULL;
        }
    }
    return;
}

thread_tptr deq(thread_tptr *head)
{
    if((*head) == NULL)
        return NULL;
    else
    {
        thread_tptr leave = (*head);
        (*head) = (*head)->th_next;
        leave->th_next = NULL;
        return leave;
    }
}
