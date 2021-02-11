#include "display.h"

struct screen* ScrInit() {
	struct screen *scr = malloc(sizeof(struct screen));
	scr->width = 128*3;
	scr->height = 64;
	scr->_framelen = 128*3*64;
	scr->_displen = 128*64;
	scr->disp[0] = SSD1306_128_64(0,0x3C);
	scr->disp[1] = SSD1306_128_64(1,0x3C);
	scr->disp[2] = SSD1306_128_64(1,0x3D);
	scr->frame = malloc(sizeof(uint8_t)*128*3*64);
	scr->layerlist = NULL;
	pthread_attr_init(&(scr->thattr));
	scr->autorefresh_runflag = 0;
	scr->need_refresh = 1;
	return scr;
}

void ScrDestroy(struct screen *scr) {
	AutoRefreshStop(scr);
	ScrRmallLayer(scr);
	free(scr->frame);
	for (int i=0;i<3;i++) SSD1306Destroy(scr->disp[i]);
	free(scr);
}

void ScrOn(struct screen *scr) {
	for (int i=0;i<3;i++) SSD1306Ctrl(scr->disp[i],DISPLAYON);
}

void ScrOff(struct screen *scr) {
	for (int i=0;i<3;i++) SSD1306Ctrl(scr->disp[i],DISPLAYOFF);
}

void ScrSetContrast(struct screen *scr,uint8_t contrast) {
	for (int i=0;i<3;i++) {
		SSD1306Ctrl(scr->disp[i],SETCONTRAST);
		SSD1306Ctrl(scr->disp[i],contrast);
	}
}

void ScrSetInversion(struct screen *scr,int invert) {
	uint8_t command;
	if (invert) command = COLORINVERT;
	else command = COLORNORMAL;
	for (int i=0;i<3;i++) SSD1306Ctrl(scr->disp[i],command);
}

void ScrAddLayer(struct screen *scr,struct gfxlayer *layer) {
	if (!scr->layerlist) {
		scr->layerlist = layer;
		layer->prev = NULL;
		layer->next = NULL;
		layer->need_refresh = &(scr->need_refresh);
	}
	else {
		layer->next = scr->layerlist;
		layer->prev = NULL;
		scr->layerlist->prev = layer;
		scr->layerlist = layer;
		layer->need_refresh = &(scr->need_refresh);
	}
}

int ScrRmLayer(struct screen *scr,struct gfxlayer *layer) {
	struct gfxlayer *pt = scr->layerlist;
	while (pt) {
		if (pt==layer) {
			if (pt->prev) pt->prev->next = pt->next;
			else scr->layerlist = pt->next;
			if (pt->next) pt->next->prev = pt->prev;
			LayerDestroy(layer);
			scr->need_refresh = 1;
			return 0;
		}
		pt = pt->next;
	}
	return -1;
}

void ScrRmallLayer(struct screen *scr) {
	struct gfxlayer *pt = scr->layerlist;
	struct gfxlayer *t;
	while (pt) {
		t = pt->next;
		LayerDestroy(pt);
		pt = t;
	}
	scr->layerlist = NULL;
}

void ScrRenderBinaryLayerBlend(struct screen *scr,struct gfxlayer *layer) {
	for (int i=MAX(0,layer->pos_x);i<MIN(scr->width,layer->pos_x+layer->width);i++)
		for (int j=MAX(0,layer->pos_y);j<MIN(scr->height,layer->pos_y+layer->height);j++)
			scr->frame[i*scr->height+j] |= layer->canvas[(i-layer->pos_x)*layer->height+j-layer->pos_y];
}

void ScrRenderBinaryLayerInvert(struct screen *scr,struct gfxlayer *layer) {
	for (int i=MAX(0,layer->pos_x);i<MIN(scr->width,layer->pos_x+layer->width);i++)
		for (int j=MAX(0,layer->pos_y);j<MIN(scr->height,layer->pos_y+layer->height);j++)
			scr->frame[i*scr->height+j] ^= layer->canvas[(i-layer->pos_x)*layer->height+j-layer->pos_y];
}

void ScrRenderBinaryLayerOveride(struct screen *scr,struct gfxlayer *layer) {
	for  (int i=MAX(0,layer->pos_x);i<MIN(scr->width,layer->pos_x+layer->width);i++)
		for (int j=MAX(0,layer->pos_y);j<MIN(scr->height,layer->pos_y+layer->height);j++)
			scr->frame[i*scr->height+j] = layer->canvas[(i-layer->pos_x)*layer->height+j-layer->pos_y];
}

void ScrRenderBinaryLayerErase(struct screen *scr,struct gfxlayer *layer) {
	for (int i=MAX(0,layer->pos_x);i<MIN(scr->width,layer->pos_x+layer->width);i++)
		for (int j=MAX(0,layer->pos_y);j<MIN(scr->height,layer->pos_y+layer->height);j++)
			scr->frame[i*scr->height+j] = !layer->canvas[(i-layer->pos_x)*layer->height+j-layer->pos_y];
}

void ScrRenderBinary(struct screen *scr) {
	struct gfxlayer *pt = scr->layerlist;
	memset(scr->frame,0,sizeof(uint8_t)*scr->_framelen);
	while (pt) {
		if (pt->rendermode==RENDER_BLEND) ScrRenderBinaryLayerBlend(scr,pt);
		else if (pt->rendermode==RENDER_INVERT) ScrRenderBinaryLayerInvert(scr,pt);
		else if (pt->rendermode==RENDER_OVERIDE) ScrRenderBinaryLayerOveride(scr,pt);
		else if (pt->rendermode==RENDER_ERASE) ScrRenderBinaryLayerErase(scr,pt);
		pt = pt->next;
	}
}

void ScrRefresh(struct screen *scr) {
	for (int i=0;i<3;i++) {
		SSD1306BufferFromArrayBinary(scr->disp[i],scr->frame+i*scr->_displen);
		SSD1306BufferFlush(scr->disp[i]);
	}
}

void* _autorefreshproc(void *arg) {
	struct screen* scr = (struct screen*) arg;
	struct timespec sleeptime;
	sleeptime.tv_sec = 0;
	sleeptime.tv_nsec = 10000;
	while (scr->autorefresh_runflag) {
		if (scr->need_refresh) {
			scr->need_refresh = 0;
			ScrRenderBinary(scr);
			ScrRefresh(scr);
		}
		nanosleep(&sleeptime,&sleeptime);
	}
	return NULL;
}

void AutoRefreshStart(struct screen *scr) {
	if (scr->autorefresh_runflag) return;
	scr->autorefresh_runflag = 1;
	pthread_create(&(scr->autorefresh_tid),&(scr->thattr),_autorefreshproc,(void*)scr);
}

void AutoRefreshStop(struct screen *scr) {
	if (!scr->autorefresh_runflag) return;
	scr->autorefresh_runflag = 0;
	pthread_join(scr->autorefresh_tid,NULL);
}