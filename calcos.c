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

int Launchapp(int appid) {
	struct gfxlayer *layer;
	if (appid==-1) {
		layer = LayerCreateBinary(0,0,128,64,RENDER_BLEND);
		ScrAddLayer(layer);
		SysFrameInit(layer);
		return 0;
	}
	else if (appid==-2) {
		sysapp_menu->window = LayerCreateBinary(128,0,128,64,RENDER_OVERIDE);
		ScrAddLayer(sysapp_menu->window);
		sysapp_menu->running = sysapp_menu->foreground = 1;
		_runningappadd(sysapp_menu);
		SysMenuInit(sysapp_menu);
		return 0;
	}
	else if (appid==4) {
		os_runflag = 0;
		return 0;
	}
	else return -1;
}

int Termapp(int appid) {
	if (appid==-1) {
		ScrRmLayer(SysFrameTerm());
		return 0;
	}
	else if (appid==-2) {
		int launchappid;
		launchappid = SysMenuTerm();
		ScrRmLayer(sysapp_menu->window);
		_runningappdel(sysapp_menu);
		memset(sysapp_menu,0,sizeof(struct apppacket));
		return launchappid;
	}
	else return -100;
}

int main(int argc,char* argv[]){
	ScrInit();
	KbdInit();
	AutoRefreshStart();
	KbdStartMonitoring();
	Launchapp(-1);
	for (int i=0;i<APPCOUNT;i++) apps[i] = calloc(0,sizeof(struct apppacket));
	sysapp_menu = malloc(sizeof(struct apppacket));
	memset(sysapp_menu,0,sizeof(struct apppacket));
	struct keyboard_event *kbdev;
	enum keyval key;
	while (os_runflag) {
		kbdev = KbdWaitEvent();
		key = keymap[kbdev->key_r][kbdev->key_c];
		if (kbdev->event_type==KEYBOARD_EVENT_KEYUP) {
			if (key==KAPP&&!sysapp_menu->running) Launchapp(-2);
			else if ((key==KAPP||key==BACK)&&sysapp_menu->running) Termapp(-2);
			else if (key==ENTR&&sysapp_menu->running) Launchapp(Termapp(-2));
			else KEHKeyUpSend(key);
		}
		else if (kbdev->event_type==KEYBOARD_EVENT_KEYDOWN) KEHKeyDownSend(key);
		KbdPostEvent();
	}
	for (int i=0;i<APPCOUNT;i++) {
		if (apps[i]&&apps[i]->running) Termapp(i);
		free(apps[i]);
	}
	Termapp(-1);
	AutoRefreshStop();
	ScrOff();
	ScrDestroy();
	KbdStopMonitoring();
	KbdDestroy();
//	system("sudo poweroff");
	return 0;
}