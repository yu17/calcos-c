#include "display.h"

struct screen* ScrInit(){
	struct screen *scr = malloc(sizeof(struct screen));
	scr->width = 128*3;
	scr->height = 64;
	scr->_framelen = 128*3*64;
	scr->_displen = 128*64;
	scr->disp[0] = SSD1306_128_64(0,0x3C);
	scr->disp[1] = SSD1306_128_64(1,0x3C);
	scr->disp[2] = SSD1306_128_64(1,0x3D);
	scr->frame = malloc(sizeof(uint8_t)*128*3*64);
	return scr;
}

void ScrDestroy(struct screen *scr){
	ScrClearLayer(scr);
	free(scr->frame);
	for (int i=0;i<3;i++) SSD1306Destroy(scr->disp[i]);
	free(scr);
}

void ScrAddLayer(struct screen *scr,struct gfxlayer *layer){
	if (scr->layerlist==NULL) {
		scr->layerlist = layer;
		layer->prev = NULL;
		layer->next = NULL;
	}
	else {
		layer->next = scr->layerlist;
		layer->prev = NULL;
		scr->layerlist->prev = layer;
		scr->layerlist = layer;
	}
}

int ScrRmLayer(struct screen *scr,struct gfxlayer *layer){
	struct gfxlayer *pt = scr->layerlist;
	while (pt) {
		if (pt==layer) {
			if (pt->prev) pt->prev->next = pt->next;
			else scr->layerlist = pt->next;
			if (pt->next) pt->next->prev = pt->prev;
			LayerDestroy(layer);
			return 0;
		}
		pt = pt->next;
	}
	return -1;
}

void ScrClearLayer(struct screen *scr){
	struct gfxlayer *pt = scr->layerlist;
	struct gfxlayer *t;
	while (pt) {
		t = pt->next;
		LayerDestroy(pt);
		pt = t;
	}
	scr->layerlist = NULL;
}

void ScrRenderBinaryLayerBlend(struct screen *scr,struct gfxlayer *layer){
	for (int i=MAX(0,layer->pos_x);i<MIN(scr->width,layer->pos_x+layer->width);i++)
		for (int j=MAX(0,layer->pos_y);j<MIN(scr->height,layer->pos_y+layer->height);j++)
			scr->frame[i*scr->height+j] |= layer->canvas[(i-layer->pos_x)*layer->height+j-layer->pos_y];
}

void ScrRenderBinaryLayerInvert(struct screen *scr,struct gfxlayer *layer){
	for (int i=MAX(0,layer->pos_x);i<MIN(scr->width,layer->pos_x+layer->width);i++)
		for (int j=MAX(0,layer->pos_y);j<MIN(scr->height,layer->pos_y+layer->height);j++)
			scr->frame[i*scr->height+j] ^= layer->canvas[(i-layer->pos_x)*layer->height+j-layer->pos_y];
}

void ScrRenderBinaryLayerOveride(struct screen *scr,struct gfxlayer *layer){
	for  (int i=MAX(0,layer->pos_x);i<MIN(scr->width,layer->pos_x+layer->width);i++)
		for (int j=MAX(0,layer->pos_y);j<MIN(scr->height,layer->pos_y+layer->height);j++)
			scr->frame[i*scr->height+j] = layer->canvas[(i-layer->pos_x)*layer->height+j-layer->pos_y];
}

void ScrRenderBinaryLayerErase(struct screen *scr,struct gfxlayer *layer){
	for (int i=MAX(0,layer->pos_x);i<MIN(scr->width,layer->pos_x+layer->width);i++)
		for (int j=MAX(0,layer->pos_y);j<MIN(scr->height,layer->pos_y+layer->height);j++)
			scr->frame[i*scr->height+j] = !layer->canvas[(i-layer->pos_x)*layer->height+j-layer->pos_y];
}

void ScrRenderBinary(struct screen *scr){
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

void ScrRefresh(struct screen *scr){
	for (int i=0;i<3;i++) {
		SSD1306BufferFromArrayBinary(scr->disp[i],scr->frame+i*scr->_displen);
		SSD1306BufferFlush(scr->disp[i]);
	}
}
