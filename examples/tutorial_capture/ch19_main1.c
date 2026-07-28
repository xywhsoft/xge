/* ch19 — 渲染线程：主线程与渲染线程协作示意 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_rect_t rc;
	int i;

	/* 主线程管道（上方） */
	rc.fX = 50; rc.fY = 100; rc.fW = 700; rc.fH = 80;
	xgeShapeRectFill(rc, XGE_COLOR_RGBA(25, 40, 55, 255));
	xgeShapeRectStroke(rc, 1.5f, XGE_COLOR_RGBA(79, 216, 194, 150));

	/* 主线程阶段块 */
	for ( i = 0; i < 4; i++ ) {
		rc.fX = 70.0f + i * 170.0f; rc.fY = 120; rc.fW = 140; rc.fH = 40;
		xgeShapeRectFill(rc, XGE_COLOR_RGBA(79, 216, 194, 180));
	}

	/* 渲染线程管道（下方） */
	rc.fX = 50; rc.fY = 280; rc.fW = 700; rc.fH = 80;
	xgeShapeRectFill(rc, XGE_COLOR_RGBA(40, 30, 20, 255));
	xgeShapeRectStroke(rc, 1.5f, XGE_COLOR_RGBA(255, 180, 84, 150));

	/* 渲染线程阶段块 */
	for ( i = 0; i < 3; i++ ) {
		rc.fX = 150.0f + i * 200.0f; rc.fY = 300; rc.fW = 160; rc.fH = 40;
		xgeShapeRectFill(rc, XGE_COLOR_RGBA(255, 180, 84, 180));
	}

	/* 命令缓冲区（连接线） */
	xgeShapeLine(400, 180, 400, 280, 2.0f, XGE_COLOR_RGBA(255, 107, 94, 200));
	xgeShapeLine(400, 180, 390, 195, 2.0f, XGE_COLOR_RGBA(255, 107, 94, 200));
	xgeShapeLine(400, 180, 410, 195, 2.0f, XGE_COLOR_RGBA(255, 107, 94, 200));

	/* 时间轴箭头 */
	xgeShapeLine(50, 450, 750, 450, 1.5f, XGE_COLOR_RGBA(200, 210, 220, 120));
	xgeShapeLine(750, 450, 735, 443, 1.5f, XGE_COLOR_RGBA(200, 210, 220, 120));
	xgeShapeLine(750, 450, 735, 457, 1.5f, XGE_COLOR_RGBA(200, 210, 220, 120));

	/* 帧标记 */
	for ( i = 0; i < 5; i++ ) {
		float x = 100.0f + i * 150.0f;
		xgeShapeLine(x, 440, x, 460, 1.0f, XGE_COLOR_RGBA(200, 210, 220, 100));
	}
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch19", argc, argv);
}
