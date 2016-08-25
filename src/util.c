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

bool timespec_AbeforeB(struct timespec a, struct timespec b)
{
    if (a.tv_sec == b.tv_sec)
        return a.tv_nsec < b.tv_nsec;
    else 
        return a.tv_sec < b.tv_sec;
}

void invalidate_L3(int* p)
{
    int i,j;
    long int sum; 
    sum =0;
    for (j=0; j<3; ++j){
        for (i = 0; i < L3_SIZE/4; i += (CACHE_LINE_SIZE/4)){
            sum += p[i]; 
        }
    }
#ifdef P4080    
    asm("msync");
#endif    
    return;
}

