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
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include "../include/define.h"
#include "../include/util.h"

extern int errno;


pthread_mutex_t cStatus_mutex;
int             consumerStatus = 0;
int             cpid = 0;    


void sig_handler(int signo)
{
  if (signo == SIGUSR1)
    consumerStatus++;

}

void initSync(){

    FILE *ptr;
    char buffer[10];

	printf("\n Producer Started ... \n");
    system("rm -rf /tmp/producer");

    system("pidof -s producer > /tmp/producer"); // Get the PID of the producer and write it to the file

    if (signal(SIGUSR1, sig_handler) == SIG_ERR){
        printf("\ncan't catch SIGINT\n");
        exit(0);
    }

    printf("\n Waiting for the consumer to start \n");
    while(consumerStatus == 0); // Wait for the consumer to start
    
    pthread_mutex_lock(&cStatus_mutex);
    consumerStatus = 0;
    pthread_mutex_unlock(&cStatus_mutex);

    printf("Consumer started with pid \n"); // Consumer has started 

    ptr = fopen("/tmp/consumer","rb");  // Get the pid of the consumer

    if(ptr == NULL){
        printf("Unable to open file \n");
        exit(0);
    }

    fread(buffer,sizeof(buffer),1,ptr); // read 10 bytes to our buffer

    cpid =  atoi(buffer); // Consumer PID

    printf("Pid of /tmp/consumer = %d \n", cpid);


}

void intConsumer(void) // send an interrupt to consumer
{
    kill(cpid, SIGUSR1);
}

void isConsumerDone(){
    while(consumerStatus == 0);
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

    int pfd;
#if MEASURE_PRODUCER    
    FILE *fp;
    char logname[100];
    int* g_mem_ptr;
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
    CPU_SET(1, &set);
    myPid = getpid();
    if (sched_setaffinity(myPid, sizeof(set), &set ) == -1){
        printf("set affinity fails for %u\n", myPid);
        return 1;
    }
                                            
//get msg size
    if (argc >= 2){
        msg_size = atoi(argv[1]) * 1024;
    }
    else{
        msg_size = 1024;
    }
//get invalid type, 'r','w', 'n'
    if (argc == 3){
        type = *(argv[2]);
    }
    else{
        type = 'n';
    }
     

    buf = (char*) malloc (msg_size);
    if (buf == NULL){
        printf ("\nmalloc error\n");
        return 1;
    }

    if(mkfifo(MYPATH, 0660) != 0) {
        printf("mkfifo");
        return 1;
    }

    pfd = open(MYPATH, O_WRONLY);
    if (pfd == -1){
        printf("open fifo");
        return 1;
    }
#if MEASURE_PRODUCER    
//init global mem for invalidate L3
    g_mem_ptr = (int*) malloc(L3_SIZE);
//log 
    logname[0] = '\0';
    strcat(logname,"./log/producer_");
    if (argc >=2) 
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
#else
#endif

    clock_gettime(CLOCK_REALTIME, &next);
    timespec_add_us(&next, PERIOD);
    printf("%s init done...\n",argv[0]);


    initSync(); 



//--------------------------
//periodic phase
//--------------------------
    for (i = 0; i <ITER; ++i) {
    
        if ( clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &next, NULL))
        {
            printf("# %s clock_nanosleep errno = %d, @%dth iter\n", argv[0], errno,i );
#if MEASURE_PRODUCER
            fprintf(fp,"# %s clock_nanosleep errno = %d, @%dth iter\n", argv[0], errno,i );
#endif            
        }
        timespec_add_us(&next, PERIOD);
        clock_gettime(CLOCK_REALTIME, &now);
        if (timespec_AbeforeB(next, now))
        {
            printf("# %s Deadline missed @%dth iter, consider rerun with looser period!\n",argv[1], i );
#if MEASURE_PRODUCER
            fprintf(fp, "# %s Deadline missed @%dth iter, consider rerun with looser period!\n",argv[1], i );
#endif            
        }

        *buf = (char)i;
        buf[msg_size-1] =(char) i;
#if MEASURE_PRODUCER
        invalidate_L3(g_mem_ptr, type);
#ifdef __P4080   
		start = photonStartTiming();
#else
        gettimeofday (&tstart, NULL);

#endif
#endif        
        write(pfd, buf, msg_size); 
#if MEASURE_PRODUCER
#ifdef __P4080   
		end = photonEndTiming();
        // release the semaphore //
		elapsed = photonReportTiming(start, end);
		photonPrintTiming(fp, elapsed);
#else
        gettimeofday(&tend, NULL);
        // release the semaphore //
        time_elapsed = (tend.tv_sec - tstart.tv_sec) * 1000000 +(tend.tv_usec - tstart.tv_usec) ; 
        
        fprintf(fp,"%lu\n", time_elapsed );
        fflush(fp);


        
#endif		
#endif
        fflush(stdout); // Write changes to the file now 
        
        intConsumer(); // Tell consumer that the data is ready by sending an interrupt signal

        isConsumerDone(); //Wait Until you hear back from the Consumer

        pthread_mutex_lock(&cStatus_mutex);
        consumerStatus = 0; 
        pthread_mutex_unlock(&cStatus_mutex);



    }
    
        
#if MEASURE_PRODUCER
    fclose (fp);
    printf("Successfully sampled %d write operations\n",ITER);
    free (g_mem_ptr);
#endif        
    free (buf);

    return 0;
}
