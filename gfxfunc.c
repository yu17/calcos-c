#include "gfxfunc.h"

#define BYTE_TO_BINARY_THRESHOLD 100
//Functions of text/font operations.
static FT_Library library;
static int ftlib_init = 1;
static int ftlib_errno;

int FontGetFaceCount(const char* fontpath) {
	if (ftlib_init) {
		ftlib_errno = FT_Init_FreeType(&library);
		if (ftlib_errno) {
			printf("Failed to initialize FreeType2 library with error %d.\n",ftlib_errno);
			return -1;
		}
		ftlib_init = 0;
	}
	FT_Face face;
	ftlib_errno = FT_New_Face(library,fontpath,-1,&face);
	if (ftlib_errno) {
		printf("Failed to load font file %s with error %d.\n",fontpath,ftlib_errno);
		return -1;
	}
	FT_Done_FreeType(library);
	return face->num_faces;
}

FT_Face FontGetFace(const char* fontpath,int faceid) {
	if (ftlib_init) {
		ftlib_errno = FT_Init_FreeType(&library);
		if (ftlib_errno) {
			printf("Failed to initialize FreeType2 library with error %d.\n",ftlib_errno);
			return NULL;
		}
		ftlib_init = 0;
	}
	FT_Face face;
	ftlib_errno = FT_New_Face(library,fontpath,faceid,&face);
	if (ftlib_errno) {
		printf("Failed to load font file %s with error %d.\n",fontpath,ftlib_errno);
		return NULL;
	}
	return face;
}

void FontDestroyFace(FT_Face font) {
	FT_Done_Face(font);
}

void FontSetSizePt(FT_Face font,int pt) {
	ftlib_errno = FT_Set_Char_Size(font,0,pt*64,150,150);
	if (ftlib_errno) printf("Failed to set font size with error %d.\n",ftlib_errno);
}

void FontSetSizePx(FT_Face font,int px) {
	ftlib_errno = FT_Set_Pixel_Sizes(font,0,px);
	if (ftlib_errno) printf("Failed to set font size with error %d.\n",ftlib_errno);
}

void FontGetStringDimension(FT_Face font,const char *str,int gap,int *width,int *height,int *bl_top,int *bl_bot) {
	*width = 0;
	*height = 0;
	*bl_top = 0;
	*bl_bot = 0;
	for (int i=0;i<strlen(str);i++) {
		ftlib_errno = FT_Load_Char(font,str[i],FT_LOAD_DEFAULT);
		if (ftlib_errno) {
			printf("Failed to load the glyph for the character %c with error %d.\n",str[i],ftlib_errno);
			return;
		}
		*width+=((font->glyph->metrics.horiAdvance>>6)+gap);
		if (font->glyph->metrics.vertAdvance>>6>*height) *height = font->glyph->metrics.vertAdvance>>6;
		if (font->glyph->bitmap_top>*bl_top) *bl_top = font->glyph->bitmap_top;
		if (font->glyph->bitmap.rows-font->glyph->bitmap_top>*bl_bot) *bl_bot = font->glyph->bitmap.rows-font->glyph->bitmap_top;
	}
	*width-=gap;
}

void FontReset(){
	FT_Done_FreeType(library);
}

struct gfxobj* ObjCreateBinaryCharPosTop(FT_Face font,const char c,int pos_x,int pos_y) {
	ftlib_errno = FT_Load_Char(font,c,FT_LOAD_RENDER);
	if (ftlib_errno) {
		printf("Failed to load the glyph for the character %c with error %d.\n",c,ftlib_errno);
		return NULL;
	}
	struct gfxobj *obj = malloc(sizeof(struct gfxobj));
	obj->pos_x = pos_x;
	obj->pos_y = pos_y;
	obj->rendermode = RENDER_BLEND;
	obj->pixelmode = PIXEL_BINARY;
	obj->visible = 1;
	FT_Bitmap *bitmap = &(font->glyph->bitmap);
	obj->width = font->glyph->metrics.horiAdvance>>6;
	obj->height = font->glyph->metrics.vertAdvance>>6;
	obj->_canvaslen = obj->width*obj->height;
	obj->canvas = malloc(sizeof(uint8_t)*obj->_canvaslen);
	memset(obj->canvas,0,obj->_canvaslen);
	for (int i=0;i<bitmap->width;i++)
		for (int j=0;j<bitmap->rows;j++) {
			if (bitmap->buffer[j*bitmap->pitch+i]>BYTE_TO_BINARY_THRESHOLD) obj->canvas[(font->glyph->bitmap_left+i)*obj->height+j] = 1;
			else obj->canvas[(font->glyph->bitmap_left+i)*obj->height+j] = 0;
		}
	return obj;
}

struct gfxobj* ObjCreateBinaryCharPosBaseline(FT_Face font,const char c,int pos_x,int pos_bl) {
	ftlib_errno = FT_Load_Char(font,c,FT_LOAD_RENDER);
	if (ftlib_errno) {
		printf("Failed to load the glyph for the character %c with error %d.\n",c,ftlib_errno);
		return NULL;
	}
	struct gfxobj *obj = malloc(sizeof(struct gfxobj));
	obj->pos_x = pos_x;
	obj->pos_y = pos_bl-font->glyph->bitmap_top;
	obj->rendermode = RENDER_BLEND;
	obj->pixelmode = PIXEL_BINARY;
	obj->visible = 1;
	FT_Bitmap *bitmap = &(font->glyph->bitmap);
	obj->width = font->glyph->metrics.horiAdvance>>6;
	obj->height = font->glyph->metrics.vertAdvance>>6;
	obj->_canvaslen = obj->width*obj->height;
	obj->canvas = malloc(sizeof(uint8_t)*obj->_canvaslen);
	memset(obj->canvas,0,obj->_canvaslen);
	for (int i=0;i<bitmap->width;i++)
		for (int j=0;j<bitmap->rows;j++) {
			if (bitmap->buffer[j*bitmap->pitch+i]>BYTE_TO_BINARY_THRESHOLD) obj->canvas[(font->glyph->bitmap_left+i)*obj->height+j] = 1;
			else obj->canvas[(font->glyph->bitmap_left+i)*obj->height+j] = 0;
		}
	return obj;
}

struct gfxobj* ObjCreateBinaryStringPosTop(FT_Face font,const char *str,int gap,int pos_x,int pos_y) {
	struct gfxobj *obj = malloc(sizeof(struct gfxobj));
	obj->pos_x = pos_x;
	obj->pos_y = pos_y;
	obj->rendermode = RENDER_BLEND;
	obj->pixelmode = PIXEL_BINARY;
	obj->visible = 1;
	int bl_top,bl_bottom;
	FontGetStringDimension(font,str,gap,&(obj->width),&(obj->height),&bl_top,&bl_bottom);
	obj->_canvaslen = obj->width*obj->height;
	obj->canvas = malloc(sizeof(uint8_t)*obj->_canvaslen);
	memset(obj->canvas,0,obj->_canvaslen);
	FT_Bitmap *bitmap;
	int cursor_i = 0;
	for (int i=0;i<strlen(str);i++) {
		ftlib_errno = FT_Load_Char(font,str[i],FT_LOAD_RENDER);
		if (ftlib_errno) {
			printf("Failed to load the glyph for the character %c with error %d.\n",str[i],ftlib_errno);
			return NULL;
		}
		bitmap = &(font->glyph->bitmap);
		for (int i=0;i<bitmap->width;i++)
			for (int j=0;j<bitmap->rows;j++) {
				if (bitmap->buffer[j*bitmap->pitch+i]>BYTE_TO_BINARY_THRESHOLD) obj->canvas[(cursor_i+font->glyph->bitmap_left+i)*obj->height+(bl_top-font->glyph->bitmap_top+j)] = 1;
				else obj->canvas[(cursor_i+font->glyph->bitmap_left+i)*obj->height+(bl_top-font->glyph->bitmap_top+j)] = 0;
			}
		cursor_i+=((font->glyph->metrics.horiAdvance>>6)+gap);
	}
	return obj;
}

struct gfxobj* ObjCreateBinaryStringPosBaseline(FT_Face font,const char *str,int gap,int pos_x,int pos_bl) {
	struct gfxobj *obj = malloc(sizeof(struct gfxobj));
	obj->pos_x = pos_x;
	obj->rendermode = RENDER_BLEND;
	obj->pixelmode = PIXEL_BINARY;
	obj->visible = 1;
	int bl_top,bl_bottom;
	FontGetStringDimension(font,str,gap,&(obj->width),&(obj->height),&bl_top,&bl_bottom);
	obj->pos_y = pos_bl-bl_top;
	obj->_canvaslen = obj->width*obj->height;
	obj->canvas = malloc(sizeof(uint8_t)*obj->_canvaslen);
	memset(obj->canvas,0,obj->_canvaslen);
	FT_Bitmap *bitmap;
	int cursor_i = 0;
	for (int i=0;i<strlen(str);i++) {
		ftlib_errno = FT_Load_Char(font,str[i],FT_LOAD_RENDER);
		if (ftlib_errno) {
			printf("Failed to load the glyph for the character %c with error %d.\n",str[i],ftlib_errno);
			return NULL;
		}
		bitmap = &(font->glyph->bitmap);
		for (int i=0;i<bitmap->width;i++)
			for (int j=0;j<bitmap->rows;j++) {
				if (bitmap->buffer[j*bitmap->pitch+i]>BYTE_TO_BINARY_THRESHOLD) obj->canvas[(cursor_i+font->glyph->bitmap_left+i)*obj->height+(bl_top-font->glyph->bitmap_top+j)] = 1;
				else obj->canvas[(cursor_i+font->glyph->bitmap_left+i)*obj->height+(bl_top-font->glyph->bitmap_top+j)] = 0;
			}
		cursor_i+=((font->glyph->metrics.horiAdvance>>6)+gap);
	}
	return obj;
}

// Functions of object operations.

// Inclusive coordinates
struct gfxobj* ObjCreateBinaryLine(int x1,int x2,int y1,int y2) {
	struct gfxobj *obj = malloc(sizeof(struct gfxobj));
	obj->pos_x = MIN(x1,x2);
	obj->pos_y = MIN(y1,y2);
	obj->width = abs(x1-x2)+1;
	obj->height = abs(y1-y2)+1;
	obj->rendermode = RENDER_BLEND;
	obj->pixelmode = PIXEL_BINARY;
	obj->visible = 1;
	obj->_canvaslen = obj->width*obj->height;
	obj->canvas = malloc(sizeof(uint8_t)*obj->_canvaslen);
	memset(obj->canvas,0,sizeof(uint8_t)*obj->_canvaslen);
	if (x1==x2)
		for (int j=0;j<obj->height;j++) obj->canvas[j]=1;
	else if (y1==y2)
		for (int i=0;i<obj->width;i++) obj->canvas[i]=1;
	else if (obj->height<=obj->width) {
		float slope = ((float)obj->height-1)/((float)obj->width-1);
		for (int i=0;i<obj->width;i++)
			obj->canvas[i*obj->height+(int)(i*slope)]=1;
	}
	else {
		float slope = ((float)obj->width-1)/((float)obj->height-1);
		for (int j=0;j<obj->height;j++)
			obj->canvas[(int)(j*slope)*obj->height+j]=1;
	}
	return obj;
}

// Inclusive cooridnates
struct gfxobj* ObjCreateBinaryRect(int x1,int x2,int y1,int y2,int filled) {
	struct gfxobj *obj = malloc(sizeof(struct gfxobj));
	obj->pos_x = MIN(x1,x2);
	obj->pos_y = MIN(y1,y2);
	obj->width = abs(x1-x2)+1;
	obj->height = abs(y1-y2)+2;
	obj->rendermode = RENDER_BLEND;
	obj->pixelmode = PIXEL_BINARY;
	obj->visible = 1;
	obj->_canvaslen = obj->width*obj->height;
	obj->canvas = malloc(sizeof(uint8_t)*obj->_canvaslen);
	if (filled) memset(obj->canvas,1,sizeof(uint8_t)*obj->_canvaslen);
	else {
		memset(obj->canvas,0,sizeof(uint8_t)*obj->_canvaslen);
		for (int i=0;i<obj->width;i++) obj->canvas[i*obj->height] = obj->canvas[(i+1)*obj->height-1] = 1;
		for (int j=0;j<obj->height;j++) obj->canvas[j] = obj->canvas[(obj->width-1)*obj->height+j] = 1;
	}
	return obj;
}

struct gfxobj* ObjCreateBinaryCircle(int x,int y,int r,int filled) {
	struct gfxobj *obj = malloc(sizeof(struct gfxobj));
	obj->pos_x = x-r+1;
	obj->pos_y = y-r+1;
	obj->width = 2*r-1;
	obj->height = 2*r-1;
	obj->rendermode = RENDER_BLEND;
	obj->pixelmode = PIXEL_BINARY;
	obj->visible = 1;
	obj->_canvaslen = obj->width*obj->height;
	obj->canvas = malloc(sizeof(uint8_t)*obj->_canvaslen);
	memset(obj->canvas,0,sizeof(uint8_t)*obj->_canvaslen);
	if (filled) {
		for (int i=0;i<obj->width;i++) {
			int yhalf = round(sqrt((r-1)*(r-1)-(r-1-i)*(r-1-i)));
			for (int j=r-1-yhalf;j<=r-1+yhalf;j++) obj->canvas[i*obj->height+j] = 1;
		}
	}
	else {
		for (int i=0;i<obj->width;i++) {
			int yhalf = round(sqrt((r-1)*(r-1)-(r-1-i)*(r-1-i)));
			obj->canvas[i*obj->height+r-1-yhalf] = 1;
			obj->canvas[i*obj->height+r-1+yhalf] = 1;
		}
		for (int j=0;j<obj->height;j++) {
			int xhalf = round(sqrt((r-1)*(r-1)-(r-1-j)*(r-1-j)));
			obj->canvas[(r-1-xhalf)*obj->height+j] = 1;
			obj->canvas[(r-1+xhalf)*obj->height+j] = 1;
		}
	}
	return obj;
}

struct gfxobj* ObjCreateBinaryPPM(const char* filepath,int pos_x,int pos_y) {
	FILE *f=fopen(filepath,"rb");
	if (!f) {
		printf("Failed to open file with errno %d!\n",errno);
		return NULL;
	}
	int width,height;
	unsigned int maxval;
	width=height=0;
	maxval=0;
	pixel **img = ppm_readppm(f,&width,&height,&maxval);
	fclose(f);
	if (width && height && maxval && img) {
		struct gfxobj *obj = malloc(sizeof(struct gfxobj));
		obj->pos_x = pos_x;
		obj->pos_y = pos_y;
		obj->width = width;
		obj->height = height;
		obj->rendermode = RENDER_BLEND;
		obj->pixelmode = PIXEL_BINARY;
		obj->visible = 1;
		obj->_canvaslen = width*height;
		obj->canvas = malloc(sizeof(uint8_t)*obj->_canvaslen);
		memset(obj->canvas,0,sizeof(uint8_t)*obj->_canvaslen);
		for (int i=0;i<width;i++)
			for (int j=0;j<height;j++)
				obj->canvas[i*height+j] = img[j][i].r>maxval/2 || img[j][i].g>maxval/2 || img[j][i].b>maxval/2;
		ppm_freearray(img,height);
		return obj;
	}
	else {
		if (height && img) ppm_freearray(img,height);
		return NULL;
	}
}

void ObjDestroy(struct gfxobj *obj) {
	free(obj->canvas);
	free(obj);
}

// Functions of layer operations.

struct gfxlayer* LayerCreateBinary(int pos_x,int pos_y,int width,int height,int rendermode) {
	struct gfxlayer *layer = malloc(sizeof(struct gfxlayer));
	layer->pos_x = pos_x;
	layer->pos_y = pos_y;
	layer->width = width;
	layer->height = height;
	layer->rendermode = rendermode;
	layer->pixelmode = PIXEL_BINARY;
	layer->visible = 1;
	layer->_canvaslen = width*height;
	layer->canvas = malloc(sizeof(uint8_t)*layer->_canvaslen);
	layer->objlist = NULL;
	layer->next = NULL;
	layer->prev = NULL;
	layer->need_refresh = NULL;
	return layer;
}

void LayerDestroy(struct gfxlayer *layer) {
	LayerClearallObj(layer);
	free(layer->canvas);
	free(layer);
}

void LayerAddObj(struct gfxlayer *layer,struct gfxobj *obj) {
	if (!layer->objlist) {
		layer->objlist = obj;
		obj->next = NULL;
		obj->prev = NULL;
	}
	else {
		obj->next = layer->objlist;
		layer->objlist->prev = obj;
		obj->prev = NULL;
		layer->objlist = obj;
	}
}

int LayerRmObj(struct gfxlayer *layer,struct gfxobj *obj) {
	struct gfxobj *pt = layer->objlist;
	while (pt) {
		if (pt==obj) {
			if (pt->prev) pt->prev->next = pt->next;
			else layer->objlist = pt->next;
			if (pt->next) pt->next->prev = pt->prev;
			return 0;
		}
		pt = pt->next;
	}
	return -1;
}

void LayerRmallObj(struct gfxlayer *layer) {
	struct gfxobj *pt = layer->objlist;
	struct gfxobj *t;
	while (pt) {
		t = pt->next;
		pt->prev = pt->next = NULL;
		pt = t;
	}
	layer->objlist = NULL;
}

int LayerClearObj(struct gfxlayer *layer,struct gfxobj *obj) {
	struct gfxobj *pt = layer->objlist;
	while (pt) {
		if (pt==obj) {
			if (pt->prev) pt->prev->next = pt->next;
			else layer->objlist = pt->next;
			if (pt->next) pt->next->prev = pt->prev;
			ObjDestroy(obj);
			return 0;
		}
		pt = pt->next;
	}
	return -1;
}

void LayerClearallObj(struct gfxlayer *layer) {
	struct gfxobj *pt = layer->objlist;
	struct gfxobj *t;
	while (pt) {
		t = pt->next;
		ObjDestroy(pt);
		pt = t;
	}
	layer->objlist = NULL;
}

void LayerClearCanvas(struct gfxlayer *layer) {
	memset(layer->canvas,0,sizeof(uint8_t)*layer->_canvaslen);
}

void LayerDirectRenderBinaryObj(struct gfxlayer *layer,struct gfxobj *obj) {
	if (obj->rendermode==RENDER_BLEND) LayerRenderBinaryObjBlend(layer,obj);
	else if (obj->rendermode==RENDER_INVERT) LayerRenderBinaryObjInvert(layer,obj);
	else if (obj->rendermode==RENDER_OVERIDE) LayerRenderBinaryObjOveride(layer,obj);
	else if (obj->rendermode==RENDER_ERASE) LayerRenderBinaryObjErase(layer,obj);
}

void LayerRenderBinaryObjBlend(struct gfxlayer *layer,struct gfxobj *obj) {
	for (int i=MAX(0,obj->pos_x);i<MIN(layer->width,obj->pos_x+obj->width);i++)
		for (int j=MAX(0,obj->pos_y);j<MIN(layer->height,obj->pos_y+obj->height);j++)
			layer->canvas[i*layer->height+j] |= obj->canvas[(i-obj->pos_x)*obj->height+j-obj->pos_y];
}

void LayerRenderBinaryObjInvert(struct gfxlayer *layer,struct gfxobj *obj) {
	for (int i=MAX(0,obj->pos_x);i<MIN(layer->width,obj->pos_x+obj->width);i++)
		for (int j=MAX(0,obj->pos_y);j<MIN(layer->height,obj->pos_y+obj->height);j++)
			layer->canvas[i*layer->height+j] ^= obj->canvas[(i-obj->pos_x)*obj->height+j-obj->pos_y];
}

void LayerRenderBinaryObjOveride(struct gfxlayer *layer,struct gfxobj *obj) {
	for  (int i=MAX(0,obj->pos_x);i<MIN(layer->width,obj->pos_x+obj->width);i++)
		for (int j=MAX(0,obj->pos_y);j<MIN(layer->height,obj->pos_y+obj->height);j++)
			layer->canvas[i*layer->height+j] = obj->canvas[(i-obj->pos_x)*obj->height+j-obj->pos_y];
}

void LayerRenderBinaryObjErase(struct gfxlayer *layer,struct gfxobj *obj) {
	for (int i=MAX(0,obj->pos_x);i<MIN(layer->width,obj->pos_x+obj->width);i++)
		for (int j=MAX(0,obj->pos_y);j<MIN(layer->height,obj->pos_y+obj->height);j++)
			layer->canvas[i*layer->height+j] = !obj->canvas[(i-obj->pos_x)*obj->height+j-obj->pos_y];
}

void LayerRenderBinary(struct gfxlayer *layer) {
	struct gfxobj *pt = layer->objlist;
	while (pt) {
		if (pt->rendermode==RENDER_BLEND) LayerRenderBinaryObjBlend(layer,pt);
		else if (pt->rendermode==RENDER_INVERT) LayerRenderBinaryObjInvert(layer,pt);
		else if (pt->rendermode==RENDER_OVERIDE) LayerRenderBinaryObjOveride(layer,pt);
		else if (pt->rendermode==RENDER_ERASE) LayerRenderBinaryObjErase(layer,pt);
		pt = pt->next;
	}
}

void LayerRefresh(struct gfxlayer *layer) {
	*(layer->need_refresh) = 1;
}
