#ifndef _DEFINE_H_
#define _DEFINE_H_

#define MEASURE_PRODUCER 1
#define ITER	(1000)
#define PERIOD 40000

#define MYPATH	"/tmp/myFIFO"
#define PRIO 1

//#define __P4080

#ifdef __P4080
#include "../include/timingUtils.h"
unsigned int * photonStartTiming();
unsigned int * photonEndTiming();
unsigned int * photonReportTiming(unsigned int* startCycles,unsigned int* endCycles);
void photonPrintTiming(FILE* fp, unsigned int * elapsed);
#endif
#define L3_SIZE (2048*1024)
#define CACHE_LINE_SIZE (64)
#endif
