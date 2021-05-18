#include "calcos.h"

#define APPCOUNT 5

static int os_runflag = 1;

char shift_flag = 0;

// Static configuration information used for starting the app.
static struct appstatic app_configs[]={
	{128,0,128,64,RENDER_OVERIDE,NULL},
	{128,0,128,64,RENDER_OVERIDE,NULL},
	{128,0,128,64,RENDER_OVERIDE,NULL},
	{128,0,128,64,RENDER_OVERIDE,NULL},
	{128,0,128,64,RENDER_OVERIDE,NULL}
};

static struct apppacket *apps[APPCOUNT];
static struct apppacket sysapp_menu;

// Uses running app stack so as to support overlays and potentially background running
static struct apppacket *running_apps = NULL;

static inline void _runningappadd(struct apppacket *app){
	if (!running_apps) {
		running_apps = app;
		app->next = NULL;
		app->prev = NULL;
	}
	else {
		running_apps->prev = app;
		app->next = running_apps;
		app->prev = NULL;
		running_apps = app;
	}
}

static inline int _runningappdel(struct apppacket *app){
	if (!app->running) return -1;
	if (app->next) app->next->prev = app->prev;
	if (app->prev) app->prev->next = app->next;
	else running_apps = app->next;
	return 0;
}

static inline int _runningapppop(struct apppacket *app) {
	if (!app->running) return -1;
	_runningappdel(app);
	_runningappadd(app);
	return 0;
}

static inline void KEHKeyUpSend(enum keyval key) {
	if (running_apps && running_apps->foreground && running_apps->keyuphandler) running_apps->keyuphandler(key);
}

static inline void KEHKeyDownSend(enum keyval key) {
	if (running_apps && running_apps->foreground && running_apps->keydownhandler) running_apps->keydownhandler(key);
}

static inline void KEHCharSend(enum keyval key) {
	if (running_apps && running_apps->foreground && running_apps->charhandler) IMKeyPress(key);
}

int Launchapp(int appid) {
	struct gfxlayer *layer;
	switch (appid) {
		case -1://SysFrame
		layer = LayerCreateBinary(0,0,128,64,RENDER_BLEND);
		ScrAddLayer(layer);
		SysFrameInit(layer);
		return 0;
		case -2://SysMenu
		if (sysapp_menu.running) break;
		sysapp_menu.window = LayerCreateBinary(128,0,128,64,RENDER_OVERIDE);
		ScrAddLayer(sysapp_menu.window);
		sysapp_menu.running = sysapp_menu.foreground = 1;
		SysMenuInit(&sysapp_menu);
		IMSetCharHandler(sysapp_menu.charhandler);
		_runningappadd(&sysapp_menu);
		return 0;
		case 1://qalc
		case 2://graph
		case 3://notebook
		case 4://terminal
		case 5://settings
		if (apps[appid]->running) break;
		apps[appid]->window = LayerCreateBinary(app_configs[appid].left,app_configs[appid].top,app_configs[appid].width,app_configs[appid].height,app_configs[appid].rendermode);
		ScrAddLayer(apps[appid]->window);
		apps[appid]->running = apps[appid]->foreground = 1;
		app_configs[appid].AppInit(apps[appid]);
		IMSetCharHandler(apps[appid]->charhandler);
		_runningappadd(apps[appid]);
		case 6://poweroff (This entry point is no longer used. SIGPWOF is preferred.)
		os_runflag = 0;
		return 0;
	}
	return -1;
}

int Termapp(int appid) {
	switch (appid) {
		case -1:
		ScrRmLayer(SysFrameTerm());
		return 0;
		case -2:
		if (!sysapp_menu.running) break;
		SysMenuTerm();
		sysapp_menu.running = sysapp_menu.foreground = 0;
		IMSetCharHandler(NULL);
		ScrRmLayer(sysapp_menu.window);
		_runningappdel(&sysapp_menu);
		memset(&sysapp_menu,0,sizeof(struct apppacket));
		return 0;
		case 1://qalc
		case 2://graph
		case 3://notebook
		case 4://terminal
		case 5://settings
		if (!apps[appid]->running) break;
		app_configs[appid].AppTerm();
		apps[appid]->running = apps[appid]->foreground = 0;
		IMSetCharHandler(NULL);
		ScrRmLayer(apps[appid]->window);
		_runningappdel(apps[appid]);
		memtset(apps[appid],0,sizeof(struct apppacket));
		default:
		return -1;
	}
	return -1;
}

//int Switchapp(int appid) {
//	if (running_apps && running_apps->foreground) {
//		running_apps->foreground = 0;
//		IMSetCharHandler(NULL);
//		ScrRmLayer(running_apps->window);
//	}
//	if (appid>=0 && apps[appid]->running) {
//		apps[appid]->foreground = 1;
//		IMSetCharHandler(apps[appid]->charhandler);
//		ScrAddLayer(apps[appid]->window);
//		_runningapppop(&(apps[appid]));
//	}
//	return 0;
//}

int OSSIG(int source,int sig,int arg) {
	switch (sig) {
		case SIGPWOF:
		os_runflag = 0;
		return 0;
		case SIGSTRT:
		Launchapp(arg);
		return 0;
		case SIGTERM:
		Termapp(source);
		return 0;
//		case SIGHANG:
//		Switchapp(-1);
//		return 0;
		default:
		return -1;
	}
}

// Main OS thread. Handles the keyboard events.
int main(int argc,char* argv[]){
	ScrInit();
	KbdInit();
	AutoRefreshStart();
	KbdStartMonitoring();
	SysIMInit();
	Launchapp(-1);
	for (int i=0;i<APPCOUNT;i++) apps[i] = calloc(0,sizeof(struct apppacket));
	memset(&sysapp_menu,0,sizeof(struct apppacket));
	struct keyboard_event *kbdev;
	enum keyval key;
	while (os_runflag) {
		kbdev = KbdWaitEvent();
		key = keymap[kbdev->key_r][kbdev->key_c];
		if (kbdev->event_type==KEYBOARD_EVENT_KEYUP) {
			if (key==KAPP&&!sysapp_menu.running) Launchapp(-2);
			else if (key==LSFT||key==RSFT) shift_flag=!shift_flag;
			else {
				KEHKeyUpSend(key);
				KEHCharSend(key);
			}
		}
		else if (kbdev->event_type==KEYBOARD_EVENT_KEYDOWN) KEHKeyDownSend(key);
		KbdPostEvent();
	}
	for (int i=0;i<APPCOUNT;i++) {
		if (apps[i]&&apps[i]->running) Termapp(i);
		free(apps[i]);
	}
	Termapp(-1);
	SysIMTerm();
	AutoRefreshStop();
	ScrOff();
	ScrDestroy();
	KbdStopMonitoring();
	KbdDestroy();
//	system("sudo poweroff");
	return 0;
}