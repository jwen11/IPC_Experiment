#ifndef _DEFINE_H_
#define _DEFINE_H_

#define MEASURE_PRODUCER 0
#define ITER	(10000)
#define PERIOD 1000

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

#endif
