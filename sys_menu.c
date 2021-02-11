#include "sys_menu.h"

#define OPTIONCOUNT 5

#define ANIMATED 0

static struct apppacket *appconf;
static struct gfxlayer *window;
static FT_Face font = NULL;
static struct gfxobj *larrow,*rarrow,*app_icon[5];
static char *app_names[5] = {"qalc","graph","terminal","settings","poweroff"};
static int selection = 0;
static struct timespec sleeptime;

void SysMenuKeyUp(enum keyval key) {
	if (key==ALFT) {
		if (selection==0) return;
		LayerRmObj(window,rarrow);
		LayerRmObj(window,larrow);
		LayerAddObj(window,app_icon[selection-1]);
		app_icon[selection-1]->pos_y = 7;
		app_icon[selection-1]->pos_x = -51;
		for (int i=0;i<8&&ANIMATED;i++) {
			app_icon[selection-1]->pos_x+=10;
			app_icon[selection]->pos_x+=10;
			LayerClearCanvas(window);
			LayerRenderBinary(window);
			nanosleep(&sleeptime,&sleeptime);
		}
		app_icon[selection-1]->pos_x=39;
		LayerRmObj(window,app_icon[selection]);
		if (selection>1) LayerAddObj(window,larrow);
		LayerAddObj(window,rarrow);
		LayerClearCanvas(window);
		LayerRenderBinary(window);
		LayerRefresh(window);
		selection--;
	}
	else if (key==ARHT) {
		if (selection==OPTIONCOUNT-1) return;
		LayerRmObj(window,rarrow);
		LayerRmObj(window,larrow);
		LayerAddObj(window,app_icon[selection+1]);
		app_icon[selection+1]->pos_y = 7;
		app_icon[selection+1]->pos_x = 129;
		for (int i=0;i<8&&ANIMATED;i++) {
			app_icon[selection+1]->pos_x-=10;
			app_icon[selection]->pos_x-=10;
			LayerClearCanvas(window);
			LayerRenderBinary(window);
			nanosleep(&sleeptime,&sleeptime);
		}
		app_icon[selection+1]->pos_x=39;
		LayerRmObj(window,app_icon[selection]);
		LayerAddObj(window,larrow);
		if (selection<OPTIONCOUNT-2) LayerAddObj(window,rarrow);
		LayerClearCanvas(window);
		LayerRenderBinary(window);
		LayerRefresh(window);
		selection++;
	}
}

void SysMenuInit(struct apppacket *app) {
	appconf = app;
	appconf->keyuphandler = SysMenuKeyUp;
	appconf->keydownhandler = NULL;
	window = appconf->window;
	selection = 0;
	sleeptime.tv_sec = 0;
	sleeptime.tv_nsec = 400000;
	font = FontGetFace("/usr/share/fonts/adobe-source-code-pro/SourceCodePro-Regular.otf",0);
	FontSetSizePx(font,18);
	larrow = ObjCreateBinaryPPM("resources/left.ppm",10,12);
	rarrow = ObjCreateBinaryPPM("resources/right.ppm",98,12);
	char fname[30];
	for (int i=0;i<OPTIONCOUNT;i++) {
		strcpy(fname,"resources/");
		strcat(fname,app_names[i]);
		strcat(fname,".ppm");
		app_icon[i] = ObjCreateBinaryPPM(fname,39,7);
	}
	LayerAddObj(window,app_icon[0]);
	LayerAddObj(window,rarrow);
	LayerClearCanvas(window);
	LayerRenderBinary(window);
	LayerRefresh(window);
}

int SysMenuTerm() {
	LayerRmallObj(window);
	for (int i=0;i<OPTIONCOUNT;i++) ObjDestroy(app_icon[i]);
	ObjDestroy(larrow);
	ObjDestroy(rarrow);
	FontDestroyFace(font);
	appconf->keyuphandler = NULL;
	return selection;
}