/* ch22 — 矩形：填充、描边、网格 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_rect_t rc;
	float x, y;

	/* 网格背景 */
	for ( x = 0; x < 800; x += 40.0f )
		xgeShapeLine(x, 0, x, 600, 0.5f, XGE_COLOR_RGBA(26, 42, 48, 64));
	for ( y = 0; y < 600; y += 40.0f )
		xgeShapeLine(0, y, 800, y, 0.5f, XGE_COLOR_RGBA(26, 42, 48, 64));

	/* 填充矩形 */
	rc.fX = 100; rc.fY = 100; rc.fW = 300; rc.fH = 200;
	xgeShapeRectFill(rc, XGE_COLOR_RGBA(79, 216, 194, 255));

	/* 半透明叠加 */
	rc.fX = 200; rc.fY = 150; rc.fW = 300; rc.fH = 200;
	xgeShapeRectFill(rc, XGE_COLOR_RGBA(255, 180, 84, 128));

	/* 描边矩形 */
	rc.fX = 150; rc.fY = 400; rc.fW = 500; rc.fH = 100;
	xgeShapeRectFill(rc, XGE_COLOR_RGBA(26, 42, 48, 255));
	xgeShapeRectStroke(rc, 1.5f, XGE_COLOR_RGBA(109, 179, 242, 255));

	/* 高亮单元格 */
	rc.fX = 120; rc.fY = 80; rc.fW = 40; rc.fH = 40;
	xgeShapeRectFill(rc, XGE_COLOR_RGBA(79, 216, 194, 48));
	xgeShapeRectStroke(rc, 1.0f, XGE_COLOR_RGBA(79, 216, 194, 255));
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch22", argc, argv);
}
