/* ch13 — 深度测试：重叠图形的遮挡关系 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_rect_t rc;

	/* 绘制三个重叠的矩形，用不同颜色表示不同"深度层" */
	/* 后层（远） */
	rc.fX = 200; rc.fY = 150; rc.fW = 250; rc.fH = 200;
	xgeShapeRectFill(rc, XGE_COLOR_RGBA(109, 179, 242, 255));

	/* 中层 */
	rc.fX = 300; rc.fY = 220; rc.fW = 250; rc.fH = 200;
	xgeShapeRectFill(rc, XGE_COLOR_RGBA(79, 216, 194, 255));

	/* 前层（近） */
	rc.fX = 400; rc.fY = 290; rc.fW = 250; rc.fH = 200;
	xgeShapeRectFill(rc, XGE_COLOR_RGBA(255, 180, 84, 255));

	/* 深度指示箭头 */
	xgeShapeLine(150, 500, 700, 500, 2.0f, XGE_COLOR_RGBA(200, 210, 220, 150));
	xgeShapeLine(700, 500, 685, 492, 2.0f, XGE_COLOR_RGBA(200, 210, 220, 150));
	xgeShapeLine(700, 500, 685, 508, 2.0f, XGE_COLOR_RGBA(200, 210, 220, 150));

	/* Z 轴标记点 */
	xgeShapeCircleFill(200, 500, 5, XGE_COLOR_RGBA(109, 179, 242, 255));
	xgeShapeCircleFill(425, 500, 5, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeShapeCircleFill(650, 500, 5, XGE_COLOR_RGBA(255, 180, 84, 255));
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch13", argc, argv);
}
