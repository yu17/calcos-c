#include "sys_frame.h"

static pthread_t mainthread;
static pthread_attr_t thattr;
static struct gfxlayer *window;
static FT_Face font;
static int runflag = 0;

void SysFrameInit(struct gfxlayer *layer) {
	window = layer;
	font = FontGetFace("/usr/share/fonts/adobe-source-code-pro/SourceCodePro-Regular.otf",0);
	FontSetSizePx(font,15);
	runflag = 1;
	pthread_attr_init(&thattr);
	pthread_create(&mainthread,&thattr,mainproc,NULL);
}

struct gfxlayer* SysFrameTerm() {
	runflag = 0;
	pthread_join(mainthread,NULL);
	FontDestroyFace(font);
	return window;
}

void* mainproc(void *args) {
	FILE *f;
	const char *fname[3] = {"/proc/net/wireless","",""};
	struct gfxobj *obj_datetime,*obj_sigtext;
	struct gfxobj *obj_sigicon = ObjCreateBinaryPPM("resources/signal.ppm",5,37);
	LayerClearallObj(window);
	LayerAddObj(window,obj_sigicon);
	float sigval_raw;
	int sigval;
	int sigval_last = -100;
	char sigstr[10];
	time_t t_sec,t_last = -100;
	struct tm *t_parsed;
	char t_str[15];
	while (runflag) {
		// Wifi signal
		f = fopen(fname[0],"r");
		fscanf(f,"%*[^\n]\n%*[^\n]\n");
		if (feof(f)) sigval = -1;
		else {
			fscanf(f,"%*[^:]: %*d %*[^.]. %f",&sigval_raw);
			sigval_raw = (sigval_raw+100)*2;
			sigval = (int)(sigval_raw<0?0:sigval_raw>100?100:sigval_raw);
		}
		fclose(f);
		// Current time
		time(&t_sec);

		// Rendering
		if (sigval!=sigval_last || t_sec/60!=t_last/60) {
			sigval_last = sigval;
			t_last = t_sec;
			LayerClearCanvas(window);
			if (sigval<0) strcpy(sigstr,"Offline");
			else snprintf(sigstr,5,"%d%%",sigval);
			obj_sigtext = ObjCreateBinaryStringPosTop(font,sigstr,0,30,45);
			LayerDirectRenderBinaryObj(window,obj_sigtext);
			ObjDestroy(obj_sigtext);
			t_parsed = localtime(&t_sec);
			strftime(t_str,15,"%b.%d %H:%M",t_parsed);
			obj_datetime = ObjCreateBinaryStringPosTop(font,t_str,0,5,5);
			LayerDirectRenderBinaryObj(window,obj_datetime);
			ObjDestroy(obj_datetime);
			LayerRenderBinary(window);
			LayerRefresh(window);
		}
		sleep(1);
	}
	LayerClearallObj(window);
	return NULL;
}
