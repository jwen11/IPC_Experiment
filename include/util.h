#ifndef __UTIL_H__
#define __UTIL_H__

/*
 * Basic utility functions.
 */

#include <time.h>
#include <stdio.h>

void timespec_add_us(struct timespec * t, long us);
struct timespec timespec_diff(struct timespec start, struct timespec end);

#endif
