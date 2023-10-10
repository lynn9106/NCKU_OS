#ifndef RESOURCE_H
#define RESOURCE_H
#include "all_include.h"
int resource_arr[8]; // 0:unavailable 1：available

void get_resources(int, int *);
void release_resources(int, int *);

#endif
