#ifndef CALCOS
#define CALCOS 0.1

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

#include "display.h"
#include "keyboard.h"
#include "calcos_api.h"
#include "sys_IM.h"

#include "sys_frame.h"
#include "sys_menu.h"

int Launchapp(int appid);

int Termapp(int appid);

#endif