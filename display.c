#include "display.h"

static int width = 128*3;
static int height = 64;
static struct SSD1306_dispconf *disp[3];
static int _displen = 128*64;
static int _framelen = 128*3*64;
static uint8_t frame[128*3*64];
static struct gfxlayer *layerlist = NULL;
pthread_t autorefresh_tid;
static pthread_attr_t thattr;
static int autorefresh_runflag = 0;
static int need_refresh = 1;

void ScrInit() {
	disp[0] = SSD1306_128_64(0,0x3C);
	disp[1] = SSD1306_128_64(1,0x3C);
	disp[2] = SSD1306_128_64(1,0x3D);
	pthread_attr_init(&thattr);
}

void ScrDestroy() {
	AutoRefreshStop();
	ScrRmallLayer();
	for (int i=0;i<3;i++) SSD1306Destroy(disp[i]);
}

void ScrOn() {
	for (int i=0;i<3;i++) SSD1306Ctrl(disp[i],DISPLAYON);
}

void ScrOff() {
	for (int i=0;i<3;i++) SSD1306Ctrl(disp[i],DISPLAYOFF);
}

void ScrSetContrast(uint8_t contrast) {
	for (int i=0;i<3;i++) {
		SSD1306Ctrl(disp[i],SETCONTRAST);
		SSD1306Ctrl(disp[i],contrast);
	}
}

void ScrSetInversion(int invert) {
	uint8_t command;
	if (invert) command = COLORINVERT;
	else command = COLORNORMAL;
	for (int i=0;i<3;i++) SSD1306Ctrl(disp[i],command);
}

void ScrAddLayer(struct gfxlayer *layer) {
	if (!layerlist) {
		layerlist = layer;
		layer->prev = NULL;
		layer->next = NULL;
		layer->need_refresh = &(need_refresh);
	}
	else {
		layer->next = layerlist;
		layer->prev = NULL;
		layerlist->prev = layer;
		layerlist = layer;
		layer->need_refresh = &(need_refresh);
	}
}

int ScrRmLayer(struct gfxlayer *layer) {
	struct gfxlayer *pt = layerlist;
	while (pt) {
		if (pt==layer) {
			if (pt->prev) pt->prev->next = pt->next;
			else layerlist = pt->next;
			if (pt->next) pt->next->prev = pt->prev;
			LayerDestroy(layer);
			need_refresh = 1;
			return 0;
		}
		pt = pt->next;
	}
	return -1;
}

void ScrRmallLayer() {
	struct gfxlayer *pt = layerlist;
	struct gfxlayer *t;
	while (pt) {
		t = pt->next;
		LayerDestroy(pt);
		pt = t;
	}
	layerlist = NULL;
}

void ScrRenderBinaryLayerBlend(struct gfxlayer *layer) {
	for (int i=MAX(0,layer->pos_x);i<MIN(width,layer->pos_x+layer->width);i++)
		for (int j=MAX(0,layer->pos_y);j<MIN(height,layer->pos_y+layer->height);j++)
			frame[i*height+j] |= layer->canvas[(i-layer->pos_x)*layer->height+j-layer->pos_y];
}

void ScrRenderBinaryLayerInvert(struct gfxlayer *layer) {
	for (int i=MAX(0,layer->pos_x);i<MIN(width,layer->pos_x+layer->width);i++)
		for (int j=MAX(0,layer->pos_y);j<MIN(height,layer->pos_y+layer->height);j++)
			frame[i*height+j] ^= layer->canvas[(i-layer->pos_x)*layer->height+j-layer->pos_y];
}

void ScrRenderBinaryLayerOveride(struct gfxlayer *layer) {
	for  (int i=MAX(0,layer->pos_x);i<MIN(width,layer->pos_x+layer->width);i++)
		for (int j=MAX(0,layer->pos_y);j<MIN(height,layer->pos_y+layer->height);j++)
			frame[i*height+j] = layer->canvas[(i-layer->pos_x)*layer->height+j-layer->pos_y];
}

void ScrRenderBinaryLayerErase(struct gfxlayer *layer) {
	for (int i=MAX(0,layer->pos_x);i<MIN(width,layer->pos_x+layer->width);i++)
		for (int j=MAX(0,layer->pos_y);j<MIN(height,layer->pos_y+layer->height);j++)
			frame[i*height+j] = !layer->canvas[(i-layer->pos_x)*layer->height+j-layer->pos_y];
}

void ScrRenderBinary() {
	struct gfxlayer *pt = layerlist;
	memset(frame,0,sizeof(uint8_t)*_framelen);
	while (pt) {
		if (pt->rendermode==RENDER_BLEND) ScrRenderBinaryLayerBlend(pt);
		else if (pt->rendermode==RENDER_INVERT) ScrRenderBinaryLayerInvert(pt);
		else if (pt->rendermode==RENDER_OVERIDE) ScrRenderBinaryLayerOveride(pt);
		else if (pt->rendermode==RENDER_ERASE) ScrRenderBinaryLayerErase(pt);
		pt = pt->next;
	}
}

void ScrRefresh() {
	for (int i=0;i<3;i++) {
		SSD1306BufferFromArrayBinary(disp[i],frame+i*_displen);
		SSD1306BufferFlush(disp[i]);
	}
}

void* _autorefreshproc(void *arg) {
	struct timespec sleeptime;
	sleeptime.tv_sec = 0;
	sleeptime.tv_nsec = 10000;
	while (autorefresh_runflag) {
		if (need_refresh) {
			need_refresh = 0;
			ScrRenderBinary();
			ScrRefresh();
		}
		nanosleep(&sleeptime,&sleeptime);
	}
	return NULL;
}

void AutoRefreshStart() {
	if (autorefresh_runflag) return;
	autorefresh_runflag = 1;
	pthread_create(&autorefresh_tid,&thattr,_autorefreshproc,NULL);
}

void AutoRefreshStop() {
	if (!autorefresh_runflag) return;
	autorefresh_runflag = 0;
	pthread_join(autorefresh_tid,NULL);
}