#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <time.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/mman.h>
#include "../include/define.h"
#include "../include/util.h"

#include <fcntl.h>

#define MAX_PATH 256
#define _STR(x) #x
#define STR(x) _STR(x)

static const char *find_debugfs(void);

int trace_fd = -1;
int marker_fd = -1;


pthread_mutex_t pStatus_mutex;
int             producerStatus = 0;
int             ppid = 0;    
extern int errno;

void sig_handler(int signo)
{
  if (signo == SIGUSR1)
    producerStatus++;

}

void initSync(){

    FILE *ptr;
    char buffer[10];



    
    system("rm -rf /tmp/consumer");

    system("pidof -s consumer > /tmp/consumer");    

    if (signal(SIGUSR1, sig_handler) == SIG_ERR){
        printf("\ncan't catch SIGINT\n");
        exit(0);
    }

    sleep(1);

    ptr = fopen("/tmp/producer","rb");  // r for read, b for binary 


    if(ptr == NULL){
        printf("Unable to open file \n");
        exit(0);
    }

    

    fread(buffer,sizeof(buffer),1,ptr); // read 10 bytes to our buffer


    
    ppid =  atoi(buffer); // Producer PID

    printf("Pid of /tmp/producer = %d \n", ppid);


}

void intProducer(void) // send an interrupt to consumer
{
    kill(ppid, SIGUSR1);
}

void waitForProducerToWrite(){
    while(producerStatus == 0);
}

int main(int argc, char** argv)
{
    
    int i;    

    struct sched_param param;
    cpu_set_t set;
    pid_t myPid;
    
    struct timespec next, now;
    unsigned long int msg_size;
    char *buf;
    char type;

// variables for finding /tracing folder
    char *debugfs;
    char path[256];


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
        printf("argc = %d\n", argc);
//lock all pages
    mlockall(MCL_CURRENT | MCL_FUTURE);        
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
    if (argc >= 2){
        msg_size = atoi(argv[1])*1024;
    }
    else{
        msg_size = 1024;
    }

    buf = (char*) malloc (msg_size);
    if (buf == NULL){
        printf ("\nmalloc error\n");
        return 1;
    }
#if BLOCKING
    pfd = open(MYPATH, O_RDONLY);
    printf("Starting the consumer with blocking read \n");
#else
    pfd = open(MYPATH, O_RDONLY | O_NONBLOCK);
    printf("Starting the consumer with non-blocking read \n");
#endif

    if (pfd == -1){
        printf("%s open fifo", argv[0]);
        return 1;
    }
//get invalid type, 'r','w', 'n'
    if (argc == 3){
        type = *(argv[2]);
    }
    else{
        type = 'n';
    }
    
#if MEASURE_PRODUCER    
#else
//init global mem for invalidate L3
    g_mem_ptr = (int*) malloc(L3_SIZE);
//log 
    logname[0] = '\0';
    strcat(logname,"./log/consumer_");
    if (argc >= 2)
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
    initSync();
    intProducer();

    clock_gettime(CLOCK_REALTIME, &next);
    timespec_add_us(&next, PERIOD);
#if MEASURE_PRODUCER    
#else    
//    timespec_add_us(&next, 3*PERIOD);
#endif    
    printf("%s init done...\n",argv[0]);

// open tracing file descriptors
    debugfs = find_debugfs();

	if (debugfs) {
	    strcpy(path, debugfs);  /* BEWARE buffer overflow */
	    strcat(path,"/tracing/tracing_on");
	    trace_fd = open(path, O_WRONLY);
	    // trace_fd = open("/sys/kernel/debug/tracing/tracing_on", O_WRONLY);
	    // printf("trace_fd is: %d\n", trace_fd);
	    
	    strcpy(path, debugfs);
	    strcat(path,"/tracing/trace_marker");
	    marker_fd = open(path, O_WRONLY);
	    // printf("marker_fd: %i\n", marker_fd);

	}


//--------------------------
//periodic phase
//--------------------------
    for (i = 0; i < ITER; ++i) {

        waitForProducerToWrite();

        pthread_mutex_lock(&pStatus_mutex);
        producerStatus = 0; 
        pthread_mutex_unlock(&pStatus_mutex);

        if ( clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &next, NULL))
        {
            printf("# %s clock_nanosleep errno = %d, @%dth iter\n", argv[0], errno,i );
#if MEASURE_PRODUCER
#else
            fprintf(fp,"# %s clock_nanosleep errno = %d, @%dth iter\n", argv[0], errno,i );
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
        invalidate_L3(g_mem_ptr, type);

        if (trace_fd >= 0)
	    {
	    	printf("trace start\n");
		    write(trace_fd, "1", 1);
	    }
#ifdef __P4080   
		start = photonStartTiming();
#else
        gettimeofday (&tstart, NULL);
#endif        
        
        printf("marker_fd:%i\n", marker_fd);
	    if (marker_fd >= 0)
	    {
	    	// printf("trace_marker works\n");
	    	write(marker_fd, "about to read\n", 14);
	    }
        
        read(pfd, buf, msg_size);

        if (marker_fd >= 0)
        	write(marker_fd, "finished read\n", 14);


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

        if (trace_fd >= 0) 
        {
		    write(trace_fd, "0", 1);
		    printf("trace finish\n");
        }
#endif       
        intProducer();// Send an interrupt to the producer
        fflush(stdout); 
#if MEASURE_PRODUCER    
#else
        fflush(fp);
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


static const char *find_debugfs(void)
{
    static char debugfs[MAX_PATH+1];
    static int debugfs_found;
    char type[100];
    FILE *fp;

    if (debugfs_found)
	    return debugfs;

    if ((fp = fopen("/proc/mounts","r")) == NULL)
	    return NULL;

    while (fscanf(fp, "%*s %"
		  STR(MAX_PATH)
		  "s %99s %*s %*d %*d\n",
		  debugfs, type) == 2) {
	    if (strcmp(type, "debugfs") == 0)
		    break;
    }
    fclose(fp);

    if (strcmp(type, "debugfs") != 0)
	    return NULL;

    debugfs_found = 1;

    return debugfs;
}


/*Read more: http://blog.fpmurphy.com/2014/05/kernel-tracing-using-ftrace.html#ixzz4IpfjUZbV*/
