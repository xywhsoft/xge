/* ch20 — 场景栈管理：栈式场景切换示意 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_rect_t rc;

	/* 场景栈可视化（从底到顶） */
	/* 底层：游戏场景 */
	rc.fX = 150; rc.fY = 380; rc.fW = 500; rc.fH = 120;
	xgeShapeRectFill(rc, XGE_COLOR_RGBA(42, 74, 63, 255));
	xgeShapeRectStroke(rc, 2.0f, XGE_COLOR_RGBA(79, 216, 194, 150));
	/* 游戏场景内容 */
	xgeShapeCircleFill(250, 440, 15, XGE_COLOR_RGBA(79, 216, 194, 200));
	xgeShapeCircleFill(400, 430, 20, XGE_COLOR_RGBA(255, 180, 84, 200));

	/* 中层：暂停菜单（半透明覆盖） */
	rc.fX = 150; rc.fY = 240; rc.fW = 500; rc.fH = 120;
	xgeShapeRectFill(rc, XGE_COLOR_RGBA(20, 25, 35, 230));
	xgeShapeRectStroke(rc, 2.0f, XGE_COLOR_RGBA(255, 180, 84, 150));
	/* 暂停菜单按钮 */
	rc.fX = 300; rc.fY = 270; rc.fW = 200; rc.fH = 30;
	xgeShapeRectFill(rc, XGE_COLOR_RGBA(50, 65, 80, 255));
	rc.fX = 300; rc.fY = 310; rc.fW = 200; rc.fH = 30;
	xgeShapeRectFill(rc, XGE_COLOR_RGBA(50, 65, 80, 255));

	/* 顶层标注 */
	rc.fX = 150; rc.fY = 100; rc.fW = 500; rc.fH = 120;
	xgeShapeRectFill(rc, XGE_COLOR_RGBA(30, 20, 40, 200));
	xgeShapeRectStroke(rc, 2.0f, XGE_COLOR_RGBA(255, 107, 94, 150));
	/* 设置面板内容 */
	rc.fX = 200; rc.fY = 120; rc.fW = 400; rc.fH = 20;
	xgeShapeRectFill(rc, XGE_COLOR_RGBA(60, 50, 70, 255));
	rc.fX = 200; rc.fY = 155; rc.fW = 400; rc.fH = 20;
	xgeShapeRectFill(rc, XGE_COLOR_RGBA(60, 50, 70, 255));
	rc.fX = 200; rc.fY = 190; rc.fW = 400; rc.fH = 20;
	xgeShapeRectFill(rc, XGE_COLOR_RGBA(60, 50, 70, 255));

	/* 栈方向箭头 */
	xgeShapeLine(700, 500, 700, 100, 2.0f, XGE_COLOR_RGBA(200, 210, 220, 150));
	xgeShapeLine(700, 100, 692, 115, 2.0f, XGE_COLOR_RGBA(200, 210, 220, 150));
	xgeShapeLine(700, 100, 708, 115, 2.0f, XGE_COLOR_RGBA(200, 210, 220, 150));
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch20", argc, argv);
}
