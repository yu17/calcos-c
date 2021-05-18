#ifndef CALCOS_API
#define CALCOS_API 0.1

#include "gfxfunc.h"
#include "keyboard.h"

#define SIGSTRT 0// Does NOT imply SIGTERM on the source. Call SIGTERM seperately.
#define SIGTERM 1
#define SIGHANG 2
#define SIGPWOF -1// Implies SIGTERM on all apps.

extern int shift_flag;

struct apppacket{
	struct gfxlayer *window;
	void (*keyuphandler)(enum keyval);
	void (*keydownhandler)(enum keyval);
	void (*charhandler)(int,char);
	int running,foreground;
	struct apppacket *next,*prev;
};

struct appstatic{
	int left,top,width,height,rendermode;
	void (*AppInit)(struct apppacket*);
	void (*AppTerm)();
};

int OSSIG(int source,int sig,int arg);

#endif