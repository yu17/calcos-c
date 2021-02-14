#ifndef CALCOS_GFXFUNC
#define CALCOS_GFXFUNC 0.1

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h> // Compile with -lm
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <ft2build.h> // Compile with -I/usr/include/freetype2 -lfreetype
#include FT_FREETYPE_H

#include <netpbm/ppm.h> // Compile with -lnetpbm

#define PIXEL_BINARY 30
#define PIXEL_GRAYSCALE 31

#define RENDER_IGNORE 20
#define RENDER_BLEND 21
#define RENDER_INVERT 22
#define RENDER_OVERIDE 23
#define RENDER_ERASE 24

#define MAX(a,b) \
	({ __typeof__ (a) _a = (a); \
	__typeof__ (b) _b = (b); \
	_a > _b ? _a : _b; })

#define MIN(a,b) \
	({ __typeof__ (a) _a = (a); \
	__typeof__ (b) _b = (b); \
	_a < _b ? _a : _b; })

struct gfxobj{
	int pos_x,pos_y,width,height,rendermode,pixelmode;
	int visible;
	int _canvaslen;
	uint8_t *canvas;
	struct gfxobj *next,*prev;
};

struct gfxlayer{
	int pos_x,pos_y,width,height,rendermode,pixelmode;
	int visible;
	int _canvaslen;
	uint8_t *canvas;
	struct gfxobj *objlist;
	struct gfxlayer *next,*prev;
	int *need_refresh;//Obtained from screen when added to it.
};

int FontGetFaceCount(const char* fontpath);

FT_Face FontGetFace(const char* fontpath,int faceid);

void FontDestroyFace(FT_Face font);

void FontSetSizePt(FT_Face font,int pt);

void FontSetSizePx(FT_Face font,int px);

void FontGetStringDimension(FT_Face font,const char *str,int gap,int *width,int *height,int *bl_top,int *bl_bot);

void FontReset();

struct gfxobj* ObjCreateBinaryCharPosTop(FT_Face font,const char c,int pos_x,int pos_y);

struct gfxobj* ObjCreateBinaryCharPosBaseline(FT_Face font,const char c,int pos_x,int pos_bl);

struct gfxobj* ObjCreateBinaryStringPosTop(FT_Face font,const char *str,int gap,int pos_x,int pos_y);

struct gfxobj* ObjCreateBinaryStringPosBaseline(FT_Face font,const char *str,int gap,int pos_x,int pos_bl);

struct gfxobj* ObjCreateBinaryLine(int x1,int x2,int y1,int y2);

struct gfxobj* ObjCreateBinaryRect(int x1,int x2,int y1,int y2,int filled);

struct gfxobj* ObjCreateBinaryCircle(int x,int y,int r,int filled);

struct gfxobj* ObjCreateBinaryPPM(const char* filepath,int pos_x,int pos_y);

void ObjFlattenBinaryObjBlend(struct gfxobj *desc,struct gfxobj *src);

void ObjFlattenBinaryObjInvert(struct gfxobj *desc,struct gfxobj *src);

void ObjFlattenBinaryObjOveride(struct gfxobj *desc,struct gfxobj *src);

void ObjFlattenBinaryObjErase(struct gfxobj *desc,struct gfxobj *src);

void ObjDestroy(struct gfxobj *obj);

struct gfxlayer* LayerCreateBinary(int pos_x,int pos_y,int width,int height,int rendermode);

struct gfxobj* ObjFlattenBinary(int objcount,...);

void LayerDestroy(struct gfxlayer *layer);

void LayerAddObj(struct gfxlayer *layer,struct gfxobj *obj);

// Use with caution. Make sure all the objects in the layer are kept track of somewhere else, or there would be memmory leakage.
int LayerRmObj(struct gfxlayer *layer,struct gfxobj *obj);

// Use with caution. Make sure all the objects in the layer are kept track of somewhere else, or there would be memmory leakage.
void LayerRmallObj(struct gfxlayer *layer);

int LayerClearObj(struct gfxlayer *layer,struct gfxobj *obj);

void LayerClearallObj(struct gfxlayer *layer);

void LayerClearCanvas(struct gfxlayer *layer);

void LayerDirectRenderBinaryObj(struct gfxlayer *layer,struct gfxobj *obj);

void LayerRenderBinaryObjBlend(struct gfxlayer *layer,struct gfxobj *obj);

void LayerRenderBinaryObjInvert(struct gfxlayer *layer,struct gfxobj *obj);

void LayerRenderBinaryObjOveride(struct gfxlayer *layer,struct gfxobj *obj);

void LayerRenderBinaryObjErase(struct gfxlayer *layer,struct gfxobj *obj);

void LayerRenderBinary(struct gfxlayer *layer);

void LayerRefresh(struct gfxlayer *layer);

#endif