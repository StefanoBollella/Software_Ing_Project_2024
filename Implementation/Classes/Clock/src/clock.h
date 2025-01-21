#ifndef clock_h
#define clock_h

#include<stdio.h>
#include<stdlib.h>
#include<iostream>
#include<cerrno>

#include<time.h>
#include<ctime>
#include<cstdio>
#include<sys/times.h>

#include "global.h"

void init_time();
void update_time();
int msleep(long msec);
int micro_sleep(long usec);
int long get_nanos(void);
long int get_day_nanos(char* buf);
long int nanos2day(char* buf, long int nanosec);

#endif // clock_h
