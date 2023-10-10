#include <stdlib.h>
#include "include/shell.h"
#include "include/command.h"
#include "include/all_include.h"

int main(int argc, char *argv[])
{
	allThreads = malloc(100 * sizeof(thread_tptr));
	th_num =0;
	algorithm=0;
	CPUidle=0;
	simuOver =0;

	ready_head =NULL;
	wait_head  =NULL;
	terminate_head =NULL;
	run =NULL;
	
	for(int i=0;i<8;i++)resource_arr[i]=1;

	Signaltimer.it_interval.tv_usec = 10000; //10ms
    Signaltimer.it_interval.tv_sec = 0;
	Signaltimer.it_value.tv_sec = 0;
    Signaltimer.it_value.tv_usec = 10000;
    signal(SIGTSTP, stopping);
	signal(SIGVTALRM, TimeHandler);

	history_count = 0;
	for (int i = 0; i < MAX_RECORD_NUM; ++i)
    	history[i] = (char *)malloc(BUF_SIZE * sizeof(char));
	if(strcmp(argv[1],"FCFS")==0)
		algorithm = 0;
	else if(strcmp(argv[1],"RR")==0)
		algorithm = 1;
	else if(strcmp(argv[1],"PP")==0)
		algorithm = 2;

	shell();

	for (int i = 0; i < MAX_RECORD_NUM; ++i)
    	free(history[i]);

	return 0;
}
