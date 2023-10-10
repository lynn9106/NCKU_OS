#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include "../include/builtin.h"
#include "../include/command.h"
#include "../include/all_include.h"

int help(char **args)
{
	int i;
	printf("--------------------------------------------------\n");
	printf("My Little Shell!!\n");
	printf("The following are built in:\n");
	for (i = 0; i < num_builtins(); i++)
	{
		printf("%d: %s\n", i, builtin_str[i]);
	}
	printf("%d: replay\n", i);
	printf("--------------------------------------------------\n");
	return 1;
}

int cd(char **args)
{
	if (args[1] == NULL)
	{
		fprintf(stderr, "lsh: expected argument to \"cd\"\n");
	}
	else
	{
		if (chdir(args[1]) != 0)
			perror("lsh");
	}
	return 1;
}

int echo(char **args)
{
	bool newline = true;
	for (int i = 1; args[i]; ++i)
	{
		if (i == 1 && strcmp(args[i], "-n") == 0)
		{
			newline = false;
			continue;
		}
		printf("%s", args[i]);
		if (args[i + 1])
			printf(" ");
	}
	if (newline)
		printf("\n");

	return 1;
}

int exit_shell(char **args)
{
	return 0;
}

int record(char **args)
{
	if (history_count < MAX_RECORD_NUM)
	{
		for (int i = 0; i < history_count; ++i)
			printf("%2d: %s\n", i + 1, history[i]);
	}
	else
	{
		for (int i = history_count % MAX_RECORD_NUM; i < history_count % MAX_RECORD_NUM + MAX_RECORD_NUM; ++i)
			printf("%2d: %s\n", i - history_count % MAX_RECORD_NUM + 1, history[i % MAX_RECORD_NUM]);
	}
	return 1;
}

bool isnum(char *str)
{
	for (int i = 0; i < strlen(str); ++i)
	{
		if (str[i] >= 48 && str[i] <= 57)
			continue;
		else
			return false;
	}
	return true;
}

int mypid(char **args)
{
	char fname[BUF_SIZE];
	char buffer[BUF_SIZE];
	if (strcmp(args[1], "-i") == 0)
	{

		pid_t pid = getpid();
		printf("%d\n", pid);
	}
	else if (strcmp(args[1], "-p") == 0)
	{

		if (args[2] == NULL)
		{
			printf("mypid -p: too few argument\n");
			return 1;
		}

		sprintf(fname, "/proc/%s/stat", args[2]);
		int fd = open(fname, O_RDONLY);
		if (fd == -1)
		{
			printf("mypid -p: process id not exist\n");
			return 1;
		}

		read(fd, buffer, BUF_SIZE);
		strtok(buffer, " ");
		strtok(NULL, " ");
		strtok(NULL, " ");
		char *s_ppid = strtok(NULL, " ");
		int ppid = strtol(s_ppid, NULL, 10);
		printf("%d\n", ppid);

		close(fd);
	}
	else if (strcmp(args[1], "-c") == 0)
	{

		if (args[2] == NULL)
		{
			printf("mypid -c: too few argument\n");
			return 1;
		}

		DIR *dirp;
		if ((dirp = opendir("/proc/")) == NULL)
		{
			printf("open directory error!\n");
			return 1;
		}

		struct dirent *direntp;
		while ((direntp = readdir(dirp)) != NULL)
		{
			if (!isnum(direntp->d_name))
			{
				continue;
			}
			else
			{
				sprintf(fname, "/proc/%s/stat", direntp->d_name);
				int fd = open(fname, O_RDONLY);
				if (fd == -1)
				{
					printf("mypid -p: process id not exist\n");
					return 1;
				}

				read(fd, buffer, BUF_SIZE);
				strtok(buffer, " ");
				strtok(NULL, " ");
				strtok(NULL, " ");
				char *s_ppid = strtok(NULL, " ");
				if (strcmp(s_ppid, args[2]) == 0)
					printf("%s\n", direntp->d_name);

				close(fd);
			}
		}

		closedir(dirp);
	}
	else
	{
		printf("wrong type! Please type again!\n");
	}

	return 1;
}

int add(char **args)
{
	int p = 0;
	char prior[20];

	if (algorithm == 2)
	{
		strcpy(prior, args[3]);
		p = atoi(prior);
	}
	TaskCreate(args[1], args[2], p);

	if (algorithm == 2 && run != NULL)			//preemptive
	{
		if (run->th_priority > p)
		{
			run->th_state = READY;
			enq(&run, &ready_head);
			run = NULL;
		}
	}

	fflush(stdout);
	return 1;
}

int del(char **args)
{
	thread_tptr target = NULL;
	thread_tptr temp_th = ready_head;
	thread_tptr ex_th = NULL;
	while (temp_th != NULL)			//search target in ready Q
	{
		if (strcmp(temp_th->th_name, args[1]) == 0)
		{
			target = temp_th;
			break;
		}
		else
		{
			ex_th = temp_th;
			temp_th = temp_th->th_next;
		}
	}
	if (target == NULL)
	{
		temp_th = wait_head;
		ex_th = NULL;
		while (temp_th != NULL)		//search target in waiting Q
		{
			if (strcmp(temp_th->th_name, args[1]) == 0)
			{
				target = temp_th;
				break;
			}
			else
			{
				ex_th = temp_th;
				temp_th = temp_th->th_next;
			}
		}
	}
	if (target == NULL)
	{
		temp_th = run;
		if (strcmp(temp_th->th_name, args[1]) == 0)	//check if the target is running task
		{
			for (int i = 0; i < 8; i++)
			{
				if (temp_th->th_resource[i] == 1)
				{
					temp_th->th_resource[i] = 0;
					resource_arr[i] = 1;
				}
			}
			target = temp_th;
			target->th_resCount =0;
			target->th_state = TERMINATED;
			target->th_next = NULL;
			printf("Task %s is killed.\n", target->th_name);
			enq(&target, &terminate_head);
			run->th_ctx = dispatch_context;
			return 1;
		}
	}
	/*if find target(not running), move it to terminate queue*/
	if (target != NULL)
	{
		for (int i = 0; i < 8; i++)
		{
			if (target->th_resource[i] == 1)
			{
				target->th_resource[i] = 0;
				resource_arr[i] = 1;
			}
		}
		target->th_resCount =0;
		/*dequeue from original queue*/
		if (target == wait_head)
			wait_head = target->th_next;
		else if (target == ready_head)
			ready_head = target->th_next;
		else
			ex_th->th_next = target->th_next;
		target->th_state = TERMINATED;
		printf("Task %s is killed.\n", target->th_name);
		target->th_next = NULL;
		enq(&target, &terminate_head); // enqueue to terminate queue
	}
	fflush(stdout);
	return 1;
}

int ps(char **args)
{

	printf("\tTID|\tname|\t   state| running| waiting| turnaround| resourse");
	if (algorithm == 2)
		printf("| priority");
	printf("\n----------------------------------------------------------------------------\n");

	for (int i = 0; i < th_num; i++)
	{
		char st[11];
		switch (allThreads[i]->th_state)
		{
		case READY:
			strcpy(st, "READY");
			break;
		case WAITING:
			strcpy(st, "WAITING");
			break;
		case TERMINATED:
			strcpy(st, "TERMINATED");
			break;
		case RUNNING:
			strcpy(st, "RUNNING");
			break;
		}

		printf("\t%3d|%8s|%11s|%8d|%8d|%11d|",
			   allThreads[i]->th_id, allThreads[i]->th_name,
			   st, allThreads[i]->th_runtime,
			   allThreads[i]->th_waittime, allThreads[i]->th_turnaroundtime);
		for (int j = 0; j < 8; j++)
		{
			if (allThreads[i]->th_resource[j] == 1)
			{
				printf(" %d", j);
			}
		}
		if (allThreads[i]->th_resCount == 0)
			printf("%9s", "none");
		if (algorithm == 2)
			printf("|\t%d ", allThreads[i]->th_priority);
		printf("\n");
	}

	return 1;
}

int start(char **args)
{
	printf("Start simulation.\n");
	getcontext(&startthread);
	if (simuOver)
	{
		simuOver = 0;
		return 1;
	}
	if (run == NULL)
	{
		CreateContext(&dispatch_context, NULL, &Dispatcher);
		setcontext(&dispatch_context);
	}
	else
	{
		setcontext(&(run->th_ctx));
	}

	return 1;
}

const char *builtin_str[] = {
	"help",
	"cd",
	"echo",
	"exit",
	"record",
	"mypid",
	"add",
	"del",
	"ps",
	"start"};

const int (*builtin_func[])(char **) = {
	&help,
	&cd,
	&echo,
	&exit_shell,
	&record,
	&mypid,
	&add,
	&del,
	&ps,
	&start};

int num_builtins()
{
	return sizeof(builtin_str) / sizeof(char *);
}
