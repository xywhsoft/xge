/* ch17 — 视口与裁剪：分屏 + 滚动裁剪 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_rect_t rc;
	int i;

	/* 左半屏：玩家 1 视角 */
	rc.fX = 10; rc.fY = 30; rc.fW = 380; rc.fH = 540;
	xgeShapeRectFill(rc, XGE_COLOR_RGBA(10, 26, 32, 255));
	xgeShapeRectStroke(rc, 2.0f, XGE_COLOR_RGBA(79, 216, 194, 150));
	/* 玩家 1 内容 */
	xgeShapeCircleFill(200, 300, 25, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeShapeRectFill((xge_rect_t){50, 450, 300, 20}, XGE_COLOR_RGBA(42, 74, 63, 255));

	/* 右半屏：玩家 2 视角 */
	rc.fX = 410; rc.fY = 30; rc.fW = 380; rc.fH = 540;
	xgeShapeRectFill(rc, XGE_COLOR_RGBA(20, 16, 10, 255));
	xgeShapeRectStroke(rc, 2.0f, XGE_COLOR_RGBA(255, 180, 84, 150));
	/* 玩家 2 内容 */
	xgeShapeCircleFill(600, 300, 25, XGE_COLOR_RGBA(255, 180, 84, 255));
	xgeShapeRectFill((xge_rect_t){450, 450, 300, 20}, XGE_COLOR_RGBA(74, 60, 42, 255));

	/* 分隔线 */
	xgeShapeLine(400, 30, 400, 570, 2.0f, XGE_COLOR_RGBA(200, 210, 220, 100));

	/* 裁剪演示区域（右下角小窗） */
	rc.fX = 500; rc.fY = 60; rc.fW = 250; rc.fH = 150;
	xgeShapeRectFill(rc, XGE_COLOR_RGBA(0, 0, 0, 150));
	/* 被裁剪的列表项（超出区域不显示） */
	for ( i = 0; i < 6; i++ ) {
		float y = 70.0f + i * 30.0f;
		if ( y + 25 > 210 ) break; /* 超出裁剪区 */
		rc.fX = 510; rc.fY = y; rc.fW = 230; rc.fH = 25;
		xgeShapeRectFill(rc, XGE_COLOR_RGBA(26, 42, 48, 255));
	}
	xgeShapeRectStroke((xge_rect_t){500, 60, 250, 150}, 1.0f, XGE_COLOR_RGBA(255, 107, 94, 180));
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch17", argc, argv);
}
