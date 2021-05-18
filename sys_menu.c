#include "sys_menu.h"

#define OPTIONCOUNT 6

#define ANIMATED 0

static struct apppacket *appconf;
static struct gfxlayer *window;
static FT_Face font = NULL;
static struct gfxobj *larrow,*rarrow,*app_icon[OPTIONCOUNT];
static char *app_names[OPTIONCOUNT] = {"qalc","graph","notebook","terminal","settings","poweroff"};
static int selection = 0;
static struct timespec sleeptime;

void SysMenuKeyUp(enum keyval key) {
	switch (key) {
		case ALFT:
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
		break;
		case ARHT:
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
		break;
		case KAPP:
		case BACK:
		OSSIG(-2,SIGTERM,0);
		break;
		case ENTR:
		OSSIG(-2,SIGTERM,0);
		if (selection!=OPTIONCOUNT-1) OSSIG(-2,SIGSTRT,selection);
		else OSSIG(-2,SIGPWOF,0);
		break;
		default:
		break;
	}
}

void SysMenuInit(struct apppacket *app) {
	appconf = app;
	appconf->keyuphandler = SysMenuKeyUp;
	appconf->keydownhandler = NULL;
	appconf->charhandler = NULL;
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
	return 0;
}