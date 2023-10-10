#ifndef THREAD_SET_H
#define THREAD_SET_H
#include"all_include.h"

typedef enum state thread_state;
enum state {
    READY,
    RUNNING,
    WAITING,
    TERMINATED
};

typedef struct thread thread_t;
typedef struct thread * thread_tptr;
struct thread
{
    char *th_name;
    int th_id;
    int th_priority;
    thread_state th_state;
    ucontext_t th_ctx;  //context recored
    struct thread *th_next;
    int th_resource[8];
    int th_resCount;
    int th_waitforRes;//if it is waiting for resources
    
    int th_runtime;    //running time
    int th_waittime;   //waiting time
    int th_turnaroundtime;     

    int th_already_wait;       //save how long you need to wait(sleep)
    int th_needtowait;          //time need to wait(sleep)
    int RR_time;      

};

thread_tptr *allThreads;
thread_tptr ready_head ;
thread_tptr wait_head ;
thread_tptr terminate_head ;
thread_tptr run;
ucontext_t mainthread;
ucontext_t startthread;


thread_tptr create_thread(char *job_name, int th_id, int priority);
void enq(thread_tptr *new_th, thread_tptr *head);
thread_tptr deq(thread_tptr *head);
#endif
