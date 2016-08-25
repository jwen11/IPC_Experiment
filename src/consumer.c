#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include "../include/define.h"
#include "../include/util.h"


extern int errno;
int main(int argc, char** argv)
{
    
    int i;    

    struct sched_param param;
    cpu_set_t set;
    pid_t myPid;
    
    struct timespec next, now;
    unsigned long int msg_size;
    char *buf;

    int pfd;
#if MEASURE_PRODUCER    
#else
    FILE *fp;
    int* g_mem_ptr;
    char logname[100];
#ifdef __P4080    
	unsigned int *start, *end, *elapsed;
#else
    struct timeval tstart, tend;    
    unsigned long int time_elapsed;
#endif
#endif    
//--------------------------
// initilization phase    
//--------------------------
//set priority
#if PRIO
    param.sched_priority = 99; /* 1(low) - 99(high) for SCHED_FIFO or SCHED_RR
                                    0 for SCHED_OTHER or SCHED_BATCH */
    if(sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
        printf("sched_setscheduler failed");
        return 1;
    }    
#endif

// set affinity
    CPU_ZERO(&set);
    CPU_SET(2, &set);
    myPid = getpid();
    if (sched_setaffinity(myPid, sizeof(set), &set ) == -1){
        printf("set affinity fails for %u\n", myPid);
        return 1;
    }
                                            
//get msg size
    if (argc == 2){
        msg_size = atoi(argv[1])*1024;
    }
    else{
        msg_size = 1024;
    }

    buf = (char*) malloc (msg_size);


    pfd = open(MYPATH, O_RDONLY);
    if (pfd == -1){
        printf("open fifo");
        return 1;
    }
    
#if MEASURE_PRODUCER    
#else
//init global mem for invalidate L3
    g_mem_ptr = (int*) malloc(L3_SIZE);
//log 
    logname[0] = '\0';
    strcat(logname,"../log/consumer_");
    if (argc == 2)
        strcat(logname,argv[1]);
    strcat(logname,".log");
    fp  = fopen (logname, "w");
    if (fp == NULL){
        printf("open log");
        return 1;
    }
    fprintf(fp,"#%s\n",logname);
#ifdef __P4080   
    fprintf(fp,"#EPU\n");
#endif
    fprintf(fp,"%d\n",ITER);
#endif    

    clock_gettime(CLOCK_REALTIME, &next);
    timespec_add_us(&next, PERIOD);
#if MEASURE_PRODUCER    
#else    
    timespec_add_us(&next, 3*PERIOD);
#endif    
    printf("%s init done...\n",argv[0]);
//--------------------------
//periodic phase
//--------------------------
    for (i = 0; i <ITER; ++i) {

        if ( clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &next, NULL))
        {
            printf("# %s clock_nanosleep errno = %d, @%dth iter\n", argv[1], errno,i );
#if MEASURE_PRODUCER
#else
            fprintf(fp,"# %s clock_nanosleep errno = %d, @%dth iter\n", argv[1], errno,i );
#endif            
        }
        timespec_add_us(&next, PERIOD);
        clock_gettime(CLOCK_REALTIME, &now);
        if (timespec_AbeforeB(next, now))
        {
            printf("# %s Deadline missed @%dth iter, consider rerun with looser period!\n",argv[1], i );
#if MEASURE_PRODUCER
#else
            fprintf(fp, "# %s Deadline missed @%dth iter, consider rerun with looser period!\n",argv[1], i );
#endif            
        }
        
#if MEASURE_PRODUCER    
        read(pfd, buf, msg_size); 
#else
        invalidate_L3(g_mem_ptr);
#ifdef __P4080   
		start = photonStartTiming();
#else
        gettimeofday (&tstart, NULL);
#endif        
        read(pfd, buf, msg_size); 
#ifdef __P4080   
		end = photonEndTiming();
		elapsed = photonReportTiming(start, end);
		photonPrintTiming(fp, elapsed);
#else
        gettimeofday(&tend, NULL);
        time_elapsed = (tend.tv_sec - tstart.tv_sec) * 1000000 +(tend.tv_usec - tstart.tv_usec) ; 
        fprintf(fp,"%lu\n", time_elapsed );
#endif        
        if ((*buf) != (char)i || buf[msg_size -1] != (char)i) {
            printf("Sanity check Failed!%d/%d != %d\n", (int)(*buf),(int) buf[msg_size -1], i);
            break;
        }
#endif        
    }
    
        

#if MEASURE_PRODUCER    
#else
    fclose (fp);
    printf("Successfully sampled %d read operations\n",ITER);
    free (g_mem_ptr);
#endif        
    free (buf);
    unlink(MYPATH);
    return 0;
}
