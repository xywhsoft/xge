/* ch101 - Render pass composition using the framework-owned outer pass. */
#include "tut_capture.h"
static void draw_scene(void)
{
	/* Each panel represents an independently configured draw list in one pass. */
	xgeShapeRectFill((xge_rect_t){85,100,270,320}, XGE_COLOR_RGBA(20,20,40,255));
	xgeShapeRectStroke((xge_rect_t){85,100,270,320}, 2.0f, XGE_COLOR_RGBA(255,107,94,255));
	xgeShapeCircleFill(220,260,90, XGE_COLOR_RGBA(255,80,80,255));
	xgeShapeRectFill((xge_rect_t){445,100,270,320}, XGE_COLOR_RGBA(20,20,40,255));
	xgeShapeRectStroke((xge_rect_t){445,100,270,320}, 2.0f, XGE_COLOR_RGBA(109,179,242,255));
	xgeShapeRectFill((xge_rect_t){520,180,120,160}, XGE_COLOR_RGBA(80,120,255,255));
	xgeShapeRectFill((xge_rect_t){135,470,530,34}, XGE_COLOR_RGBA(31,46,61,255));
}
int main(int argc, char** argv) { return tut_run(draw_scene, "ch101-render-pass", argc, argv); }
