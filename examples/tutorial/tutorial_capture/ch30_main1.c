/* ch30 — 圆角矩形（进阶）：四角独立半径 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_rect_t rc;
	xge_shape_round_rect_t radii;

	/* 仅上方圆角（卡片头部） */
	radii.fTopLeft = 16; radii.fTopRight = 16;
	radii.fBottomRight = 0; radii.fBottomLeft = 0;
	rc.fX = 100; rc.fY = 80; rc.fW = 300; rc.fH = 60;
	xgeShapeRoundRectFillEx(rc, radii, XGE_SHAPE_ROUND_RECT_AUTO,
		XGE_COLOR_RGBA(42, 74, 80, 255));
	/* 下方主体 */
	rc.fX = 100; rc.fY = 140; rc.fW = 300; rc.fH = 140;
	xgeShapeRectFill(rc, XGE_COLOR_RGBA(26, 42, 48, 255));

	/* 对角圆角（标签样式） */
	radii.fTopLeft = 12; radii.fTopRight = 0;
	radii.fBottomRight = 12; radii.fBottomLeft = 0;
	rc.fX = 500; rc.fY = 100; rc.fW = 150; rc.fH = 45;
	xgeShapeRoundRectFillEx(rc, radii, XGE_SHAPE_ROUND_RECT_SDF,
		XGE_COLOR_RGBA(255, 180, 84, 255));

	/* 大圆角 SDF */
	rc.fX = 200; rc.fY = 380; rc.fW = 400; rc.fH = 150;
	xgeShapeRoundRectFill(rc, 40.0f, XGE_COLOR_RGBA(79, 216, 194, 200));
	xgeShapeRoundRectStroke(rc, 40.0f, 2.0f, XGE_COLOR_RGBA(255, 255, 255, 100));
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch30", argc, argv);
}
