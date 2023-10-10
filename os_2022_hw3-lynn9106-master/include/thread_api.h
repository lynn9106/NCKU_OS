#ifndef THREAD_API_H
#define THREAD_API_H

#include "all_include.h"

#define STACK_SIZE 10240
ucontext_t dispatch_context; // dispatcher context
int TaskCreate(char *job_name, char *p_function, int priority);
void CreateContext(ucontext_t *context, ucontext_t *next_context, void *func);
void Dispatcher();
void stopping(int sig);
void TimeHandler(int sig);
int CPUidle;
int th_num;
int simuOver;
struct itimerval Signaltimer;
#endif