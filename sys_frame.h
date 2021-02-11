#ifndef CALCOS_SYS_FRAME
#define CALCOS_SYS_FRAME 0.1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h> // Compile with -lpthread
#include <semaphore.h> // Compile with -lpthread

#include "gfxfunc.h"
#include "calcos_api.h"

void SysFrameInit(struct gfxlayer *layer);

struct gfxlayer* SysFrameTerm();

void* mainproc(void *args);

#endif