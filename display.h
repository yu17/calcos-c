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

#include "SSD1306.h"
#include "gfxfunc.h"

struct screen{
	int width,height;
	struct SSD1306_dispconf * disp[3];
	int _displen,_framelen;
	uint8_t *frame;
	struct gfxlayer *layerlist;
};

struct screen* ScrInit();

void ScrDestroy(struct screen *scr);

void ScrAddLayer(struct screen *scr,struct gfxlayer *layer);

int ScrRmLayer(struct screen *scr,struct gfxlayer *layer);

void ScrClearLayer(struct screen *scr);

void ScrRenderBinaryLayerBlend(struct screen *scr,struct gfxlayer *layer);

void ScrRenderBinaryLayerInvert(struct screen *scr,struct gfxlayer *layer);

void ScrRenderBinaryLayerOveride(struct screen *scr,struct gfxlayer *layer);

void ScrRenderBinaryLayerErase(struct screen *scr,struct gfxlayer *layer);

void ScrRenderBinary(struct screen *scr);

void ScrRefresh(struct screen *scr);

#endif