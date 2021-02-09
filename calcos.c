#include "display.h"
#include "keyboard.h"

int main(){
	struct screen *scr = ScrInit();
	struct gfxlayer *layer = LayerCreateBinary(0,0,128,64,RENDER_BLEND);
	ScrAddLayer(scr,layer);
	struct gfxobj *obj = ObjCreateBinaryPPM("resources/left.ppm",0,12);
	LayerAddObj(layer,obj);
	obj = ObjCreateBinaryPPM("resources/right.ppm",108,12);
	LayerAddObj(layer,obj);
	FT_Face font = FontGetFace("DejaVuSans.ttf",0);
	FontSetSizePx(font,20);
	obj = ObjCreateBinaryStringPosBaseline(font,"xyZ",0,30,25);
	LayerAddObj(layer,obj);
	LayerRenderBinary(layer);
	ScrRenderBinary(scr);
	ScrRefresh(scr);
	struct keyboard *kbd = KbdInit();
	KbdStartMonitoring(kbd);
	ScrDestroy(scr);
	KbdDestroy(kbd);
	return 0;
}