#ifndef CALCOS_DISPLAY
#define CALCOS_DISPLAY 0.1

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

#include "SSD1306.h"
#include "gfxfunc.h"

void ScrInit();

void ScrDestroy();

void ScrOn();

void ScrOff();

void ScrSetContrast(uint8_t contrast);

void ScrSetInversion(int invert);

// Append the layer to the screen.
void ScrAddLayer(struct gfxlayer *layer);

// Remove the layer from the screen and also destroy it.
int ScrRmLayer(struct gfxlayer *layer);

void ScrRmallLayer();

void ScrRenderBinaryLayerBlend(struct gfxlayer *layer);

void ScrRenderBinaryLayerInvert(struct gfxlayer *layer);

void ScrRenderBinaryLayerOveride(struct gfxlayer *layer);

void ScrRenderBinaryLayerErase(struct gfxlayer *layer);

void ScrRenderBinary();

void ScrRefresh();

void* _autorefreshproc(void *arg);

void AutoRefreshStart();

void AutoRefreshStop();

#endif