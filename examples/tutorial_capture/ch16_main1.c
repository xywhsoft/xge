/* ch16 — 坐标转换：世界坐标与屏幕坐标 + 小地图 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_rect_t rc;

	/* 主世界区域 */
	rc.fX = 20; rc.fY = 20; rc.fW = 560; rc.fH = 560;
	xgeShapeRectFill(rc, XGE_COLOR_RGBA(15, 25, 30, 255));
	xgeShapeRectStroke(rc, 1.5f, XGE_COLOR_RGBA(100, 120, 140, 120));

	/* 世界中的物体 */
	xgeShapeCircleFill(200, 250, 15, XGE_COLOR_RGBA(255, 180, 84, 255));
	xgeShapeCircleFill(400, 350, 15, XGE_COLOR_RGBA(79, 216, 194, 255));
	rc.fX = 100; rc.fY = 400; rc.fW = 80; rc.fH = 60;
	xgeShapeRectFill(rc, XGE_COLOR_RGBA(109, 179, 242, 200));

	/* 鼠标拾取线（屏幕→世界） */
	xgeShapeLine(580, 100, 200, 250, 1.0f, XGE_COLOR_RGBA(255, 107, 94, 150));
	xgeShapeCircleFill(580, 100, 4, XGE_COLOR_RGBA(255, 107, 94, 255));

	/* 小地图（右下角） */
	rc.fX = 600; rc.fY = 400; rc.fW = 180; rc.fH = 180;
	xgeShapeRectFill(rc, XGE_COLOR_RGBA(0, 0, 0, 200));
	xgeShapeRectStroke(rc, 1.5f, XGE_COLOR_RGBA(79, 216, 194, 150));

	/* 小地图中的点 */
	xgeShapeCircleFill(640, 470, 3, XGE_COLOR_RGBA(255, 180, 84, 255));
	xgeShapeCircleFill(700, 500, 3, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeShapeCircleFill(620, 520, 3, XGE_COLOR_RGBA(109, 179, 242, 255));

	/* 视口框（小地图中当前可见区域） */
	rc.fX = 620; rc.fY = 450; rc.fW = 60; rc.fH = 50;
	xgeShapeRectStroke(rc, 1.0f, XGE_COLOR_RGBA(255, 255, 255, 150));
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch16", argc, argv);
}
