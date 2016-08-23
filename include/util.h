#ifndef __UTIL_H__
#define __UTIL_H__

/*
 * Basic utility functions.
 */

#include <time.h>
#include <stdio.h>

#include "define.h"

void timespec_add_us(struct timespec * t, long us);
struct timespec timespec_diff(struct timespec start, struct timespec end);
void invalidate_L3(int* p);
#endif
