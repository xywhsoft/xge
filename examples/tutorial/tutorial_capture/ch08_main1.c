/* ch08 — 帧统计与调试：HUD 性能面板 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_rect_t rc;
	int i;

	/* 模拟场景：多个随机圆形 */
	xgeShapeCircleFill(200, 200, 60, XGE_COLOR_RGBA(79, 216, 194, 180));
	xgeShapeCircleFill(500, 350, 80, XGE_COLOR_RGBA(255, 180, 84, 150));
	xgeShapeCircleFill(650, 150, 40, XGE_COLOR_RGBA(255, 107, 94, 200));
	xgeShapeCircleFill(350, 450, 50, XGE_COLOR_RGBA(109, 179, 242, 160));

	/* HUD 面板背景 */
	rc.fX = 20; rc.fY = 20; rc.fW = 320; rc.fH = 130;
	xgeShapeRectFill(rc, XGE_COLOR_RGBA(0, 0, 0, 180));
	xgeShapeRectStroke(rc, 1.0f, XGE_COLOR_RGBA(79, 216, 194, 120));

	/* 帧时间条形图（模拟最近几帧） */
	for ( i = 0; i < 12; i++ ) {
		float h = 10.0f + (float)((i * 7 + 3) % 11) * 3.0f;
		float x = 30.0f + i * 22.0f;
		uint32_t col = (h > 30.0f) ?
			XGE_COLOR_RGBA(255, 107, 94, 220) :
			XGE_COLOR_RGBA(79, 216, 194, 220);
		rc.fX = x; rc.fY = 130.0f - h; rc.fW = 16; rc.fH = h;
		xgeShapeRectFill(rc, col);
	}

	/* 16.6ms 参考线 */
	xgeShapeLine(30, 130.0f - 16.6f, 294, 130.0f - 16.6f, 1.0f, XGE_COLOR_RGBA(255, 180, 84, 180));
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch08", argc, argv);
}
