/* ch28 — 胶囊形：按钮 + 进度条 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_rect_t rc;

	/* 水平胶囊按钮 */
	rc.fX = 300; rc.fY = 100; rc.fW = 200; rc.fH = 50;
	xgeShapeCapsuleFill(rc, XGE_COLOR_RGBA(79, 216, 194, 255));

	/* 描边胶囊 */
	rc.fX = 300; rc.fY = 180; rc.fW = 200; rc.fH = 50;
	xgeShapeCapsuleStroke(rc, 2.0f, XGE_COLOR_RGBA(255, 180, 84, 255));

	/* 进度条 */
	rc.fX = 100; rc.fY = 300; rc.fW = 600; rc.fH = 24;
	xgeShapeCapsuleFill(rc, XGE_COLOR_RGBA(51, 51, 51, 255));
	rc.fX = 100; rc.fY = 300; rc.fW = 390; rc.fH = 24;
	xgeShapeCapsuleFill(rc, XGE_COLOR_RGBA(79, 216, 194, 255));

	/* 垂直胶囊 */
	rc.fX = 150; rc.fY = 380; rc.fW = 40; rc.fH = 160;
	xgeShapeCapsuleFill(rc, XGE_COLOR_RGBA(255, 107, 94, 255));

	/* 正方形 → 圆形 */
	rc.fX = 250; rc.fY = 400; rc.fW = 80; rc.fH = 80;
	xgeShapeCapsuleFill(rc, XGE_COLOR_RGBA(109, 179, 242, 255));
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch28", argc, argv);
}
