#ifndef _MARKJOB_TYPES_H
#define _MARKJOB_TYPES_H

#ifndef itoa
#define itoa(a,b,c) sprintf(b,"%d",a)
#endif


#include <sys/types.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <pthread.h>

#ifndef LPVOID
#define LPVOID void *
#endif

#ifndef __int64 
#define __int64 int64_t
#endif

#define msleep(x) usleep(1000*x)


#include <time.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string>
#include <memory.h>
#include <ctype.h>
#include <vector>

using namespace std;


#endif