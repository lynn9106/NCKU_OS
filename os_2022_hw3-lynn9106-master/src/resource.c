#include "../include/resource.h"
#include "../include/all_include.h"

void get_resources(int count, int *resources)
{
        getcontext(&(run->th_ctx));     //record the context

        for(int i=0;i<count;i++){
            if(resources[i]>7) return;
            if(!resource_arr[resources[i]]){            //if there is resource unavailable
                printf("Task %s is waiting resource.\n",run->th_name);
                run->RR_time=0;
                for(int j=0;j<count;j++)run->th_resource[resources[j]] = 2;
                run->th_waitforRes=1;
                run->th_state = WAITING;
                enq(&run,&wait_head);                   //make the task goto waiting queue
                setcontext(&dispatch_context);          //dispatch next context
            }
        }

        for(int i=0;i<count;i++){                       //all the resources are available
                if(resource_arr[resources[i]]){
                        run->th_resource[resources[i]] =1;             //take the resource
                        resource_arr[resources[i]]=0;
                        printf("Task %s gets resource %d.\n",run->th_name,resources[i]);
                }
        }
        run->th_resCount += count;
        run->th_waitforRes=0;   //no need to wait for resource
}

void release_resources(int count, int *resources)
{
        for(int i=0;i<count;i++){
                printf("Task %s releases resource %d.\n",run->th_name,resources[i]);
                run->th_resource[resources[i]] = 0;     //release the rsource, resource is available
                resource_arr[resources[i]]=1;
        }
        run->th_resCount -= count;
}
