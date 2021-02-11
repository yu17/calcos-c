#ifndef CALCOS_SYS_MENU
#define CALCOS_SYS_MENU 0.1

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

#include "gfxfunc.h"
#include "keyboard.h"
#include "calcos_api.h"

void SysMenuKeyUp(enum keyval key);

void SysMenuInit(struct apppacket *app);

int SysMenuTerm();

#endif