#include "../include/util.h"

void timespec_add_us(struct timespec * t, long us)
{
	t->tv_nsec += us * 1000;
	if(t->tv_nsec > 1000000000UL) {
		t->tv_nsec = t->tv_nsec - 1000000000UL;
		t->tv_sec += 1;
	}
}

struct timespec timespec_diff(struct timespec start, struct timespec end)
{
	struct timespec temp;
	if ((end.tv_nsec-start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;
}

void invalidate_L3(int* p)
{
    int i;
    long int sum; 
    sum =0;
    for (i = 0; i < L3_SIZE; i += CACHE_LINE_SIZE/4){
        sum += p[i]; 
    }
}
