/* ch97 - Render target usage.  tut_capture provides the real outer target. */
#include "tut_capture.h"
static void draw_scene(void)
{
	/* Do not create a nested pass inside the capture framework's active pass. */
	xgeShapeRectFill((xge_rect_t){90,80,620,440}, XGE_COLOR_RGBA(26,42,57,255));
	xgeShapeRectStroke((xge_rect_t){90,80,620,440}, 3.0f, XGE_COLOR_RGBA(109,179,242,255));
	xgeShapeCircleFill(260,285,112, XGE_COLOR_RGBA(255,180,84,255));
	xgeShapeRectFill((xge_rect_t){410,185,190,190}, XGE_COLOR_RGBA(79,216,194,180));
	xgeShapeRectStroke((xge_rect_t){410,185,190,190}, 2.0f, XGE_COLOR_RGBA(79,216,194,255));
	xgeShapeRectFill((xge_rect_t){255,430,290,46}, XGE_COLOR_RGBA(18,27,36,255));
}
int main(int argc, char** argv) { return tut_run(draw_scene, "ch97-render-target", argc, argv); }
