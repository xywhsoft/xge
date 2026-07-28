/* ch02 — 引擎初始化与窗口创建 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_rect_t rc;

	/* 深色背景已由 pass clear 绘制 */

	/* 模拟窗口边框 */
	rc.fX = 100; rc.fY = 60; rc.fW = 600; rc.fH = 480;
	xgeShapeRectStroke(rc, 2.0f, XGE_COLOR_RGBA(79, 216, 194, 255));

	/* 标题栏区域 */
	rc.fX = 100; rc.fY = 60; rc.fW = 600; rc.fH = 36;
	xgeShapeRectFill(rc, XGE_COLOR_RGBA(30, 40, 50, 255));

	/* 标题栏小圆点（窗口按钮） */
	xgeShapeCircleFill(120, 78, 6, XGE_COLOR_RGBA(255, 107, 94, 255));
	xgeShapeCircleFill(140, 78, 6, XGE_COLOR_RGBA(255, 180, 84, 255));
	xgeShapeCircleFill(160, 78, 6, XGE_COLOR_RGBA(79, 216, 194, 255));

	/* 客户区中心标识 */
	xgeShapeCircleFill(400, 320, 40, XGE_COLOR_RGBA(255, 180, 84, 180));
	xgeShapeCircleStroke(400, 320, 55, 2.0f, XGE_COLOR_RGBA(79, 216, 194, 200));

	/* 尺寸标注线 */
	xgeShapeLine(100, 560, 700, 560, 1.5f, XGE_COLOR_RGBA(100, 120, 140, 255));
	xgeShapeLine(720, 60, 720, 540, 1.5f, XGE_COLOR_RGBA(100, 120, 140, 255));
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch02", argc, argv);
}
