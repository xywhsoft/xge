/* ch29 — 圆角矩形（基础）：卡片 + 不同半径 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_rect_t rc;
	float radii[] = { 0, 4, 8, 16, 25 };
	int i;

	/* 卡片背景 */
	rc.fX = 100; rc.fY = 80; rc.fW = 300; rc.fH = 200;
	xgeShapeRoundRectFill(rc, 12.0f, XGE_COLOR_RGBA(26, 42, 48, 255));
	xgeShapeRoundRectStroke(rc, 12.0f, 1.5f, XGE_COLOR_RGBA(79, 216, 194, 96));

	/* Draw 一次完成 */
	rc.fX = 500; rc.fY = 80; rc.fW = 180; rc.fH = 50;
	xgeShapeRoundRectDraw(rc, 8.0f,
		XGE_COLOR_RGBA(42, 58, 64, 255), 2.0f, XGE_COLOR_RGBA(255, 180, 84, 255));

	/* 不同半径对比 */
	for ( i = 0; i < 5; i++ ) {
		rc.fX = 80.0f + i * 140.0f; rc.fY = 380; rc.fW = 120; rc.fH = 80;
		xgeShapeRoundRectFill(rc, radii[i], XGE_COLOR_RGBA(79, 216, 194, 255));
	}
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch29", argc, argv);
}
