#include "calcos.h"

#define APPCOUNT 4

static int os_runflag = 1;

static struct apppacket *apps[APPCOUNT];
static struct apppacket *sysapp_menu;

static struct apppacket *running_apps = NULL;

void _runningappadd(struct apppacket *app){
	if (!running_apps) {
		running_apps = app;
		app->next = NULL;
	}
	else {
		app->next = running_apps;
		running_apps = app;
	}
}

int _runningappdel(struct apppacket *app){
	struct apppacket *pt = running_apps;
	if (pt==app) {
		running_apps = pt->next;
		return 0;
	}
	while (pt->next) {
		if (pt->next==app) {
			pt->next = pt->next->next;
			return 0;
		}
		pt = pt->next;
	}
	return -1;
}

void KEHKeyUpSend(enum keyval key) {
	struct apppacket *pt = running_apps;
	while (pt) {
		if (pt->foreground && pt->keyuphandler) {
			pt->keyuphandler(key);
			break;
		}
		pt = pt->next;
	}
}

void KEHKeyDownSend(enum keyval key) {
	struct apppacket *pt = running_apps;
	while (pt) {
		if (pt->foreground && pt->keydownhandler) {
			pt->keydownhandler(key);
			break;
		}
		pt = pt->next;
	}
}

int Launchapp(int appid,struct screen *scr) {
	struct gfxlayer *layer;
	if (appid==-1) {
		layer = LayerCreateBinary(0,0,128,64,RENDER_BLEND);
		ScrAddLayer(scr,layer);
		SysFrameInit(layer);
		return 0;
	}
	else if (appid==-2) {
		sysapp_menu->window = LayerCreateBinary(128,0,128,64,RENDER_OVERIDE);
		ScrAddLayer(scr,sysapp_menu->window);
		sysapp_menu->running = sysapp_menu->foreground = 1;
		_runningappadd(sysapp_menu);
		SysMenuInit(sysapp_menu);
		return 0;
	}
	else return -1;
}

int Termapp(int appid,struct screen *scr) {
	if (appid==-1) {
		ScrRmLayer(scr,SysFrameTerm());
		return 0;
	}
	else if (appid==-2) {
		int launchappid;
		launchappid = SysMenuTerm();
		ScrRmLayer(scr,sysapp_menu->window);
		_runningappdel(sysapp_menu);
		memset(sysapp_menu,0,sizeof(struct apppacket));
		return launchappid;
	}
	else return -100;
}

int main(int argc,char* argv[]){
	struct screen *scr = ScrInit();
	struct keyboard *kbd = KbdInit();
	AutoRefreshStart(scr);
	KbdStartMonitoring(kbd);
	Launchapp(-1,scr);
	for (int i=0;i<APPCOUNT;i++) apps[i] = calloc(0,sizeof(struct apppacket));
	sysapp_menu = malloc(sizeof(struct apppacket));
	memset(sysapp_menu,0,sizeof(struct apppacket));
	struct keyboard_event *kbdevent;
	enum keyval key;
	while (os_runflag) {
		kbdevent = KbdWaitEvent(kbd);
		if (kbdevent) {
			key = keymap[kbdevent->key_r][kbdevent->key_c];
			if (kbdevent->event_type == KEYBOARD_EVENT_KEYUP) {
				if (key==KAPP&&!sysapp_menu->running) Launchapp(-2,scr);
				else if ((key==KAPP||key==BACK)&&sysapp_menu->running) Termapp(-2,scr);
				else if (key==ENTR&&sysapp_menu->running) Launchapp(Termapp(-2,scr),scr);
				else KEHKeyUpSend(key);
			}
			else if (kbdevent->event_type == KEYBOARD_EVENT_KEYDOWN) KEHKeyDownSend(key);
			free(kbdevent);
		}
	}
	for (int i=0;i<APPCOUNT;i++) {
		if (apps[i]&&apps[i]->running) Termapp(i,scr);
		free(apps[i]);
	}
	Termapp(-1,scr);
	AutoRefreshStop(scr);
	ScrOff(scr);
	ScrDestroy(scr);
	KbdStopMonitoring(kbd);
	KbdDestroy(kbd);
	return 0;
}