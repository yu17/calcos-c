#ifndef CALCOS_SYS_IM
#define CALCOS_SYS_IM 0.1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <pthread.h> // Compile with -lpthread
#include <semaphore.h> // Compile with -lpthread

#include "calcos_api.h"

#define IM_PENDING 60
#define IM_SWITCH 61
#define IM_DETERMINED 62
#define IM_NONHYBRID 63

static inline void IMSetCharHandler(void (*charhandler)(int,char));

// Should be called only once by the OS. Should not be called by any apps.
void SysIMInit();

// Should be called only once by the OS. Should not be called by any apps.
void SysIMTerm();

static inline void IMKeyPress(enum keyval key);

void *_immainproc(void *arg);

#endif