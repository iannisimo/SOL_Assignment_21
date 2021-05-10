#include <time.h>
#include "utils.h"

#ifndef __connection_h
#define __connection_h

int openConnection(const char *sockname, int msec, const struct timespec abstime);
int closeConnection(const char *sockname);


#endif