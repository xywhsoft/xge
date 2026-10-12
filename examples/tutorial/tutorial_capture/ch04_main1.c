/* ch04 — 手动渲染模式：离屏渲染圆形 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_rect_t rc;

	/* 大圆形（教程示例中的琥珀色圆） */
	xgeShapeCircleFill(400, 300, 150, XGE_COLOR_RGBA(255, 180, 84, 255));

	/* 内圈装饰 */
	xgeShapeCircleStroke(400, 300, 100, 2.0f, XGE_COLOR_RGBA(13, 17, 22, 180));
	xgeShapeCircleStroke(400, 300, 50, 2.0f, XGE_COLOR_RGBA(13, 17, 22, 120));

	/* 中心点 */
	xgeShapeCircleFill(400, 300, 8, XGE_COLOR_RGBA(13, 17, 22, 200));

	/* 边框表示离屏帧缓冲 */
	rc.fX = 50; rc.fY = 30; rc.fW = 700; rc.fH = 540;
	xgeShapeRectStroke(rc, 1.5f, XGE_COLOR_RGBA(100, 120, 140, 150));
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch04", argc, argv);
}
