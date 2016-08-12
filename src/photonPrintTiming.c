/********************************
Author: Sravanthi Kota Venkata
********************************/

/** C File **/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "../include/timingUtils.h"

void photonPrintTiming(FILE* fp, unsigned int * elapsed)
{
    if(elapsed[1] == 0)
	    fprintf(fp,"%u\n", elapsed[0]);
    else
	    fprintf(fp,"%u%u\n", elapsed[1], elapsed[0]);
}

/** End of C Code **/
