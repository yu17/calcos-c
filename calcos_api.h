#ifndef CALCOS_API
#define CALCOS_API 0.1

#include "gfxfunc.h"
#include "keyboard.h"

struct apppacket{
	struct gfxlayer *window;
	void (*keyuphandler)(enum keyval);
	void (*keydownhandler)(enum keyval);
	int running,foreground;
	struct apppacket *next;
};

#endif