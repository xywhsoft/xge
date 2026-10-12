/* ch05 — 窗口管理：自适应居中圆形 */
#include "tut_capture.h"

static void draw_scene(void)
{
	float cx = 400.0f, cy = 300.0f;
	float r = 180.0f; /* 短边 600 * 0.3 = 180 */

	/* 主圆形（青绿色填充） */
	xgeShapeCircleFill(cx, cy, r, XGE_COLOR_RGBA(79, 216, 194, 255));

	/* 琥珀色描边（r+10） */
	xgeShapeCircleStroke(cx, cy, r + 10, 3.0f, XGE_COLOR_RGBA(255, 180, 84, 255));

	/* 十字准线标识中心 */
	xgeShapeLine(cx - 20, cy, cx + 20, cy, 1.5f, XGE_COLOR_RGBA(13, 17, 22, 180));
	xgeShapeLine(cx, cy - 20, cx, cy + 20, 1.5f, XGE_COLOR_RGBA(13, 17, 22, 180));
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch05", argc, argv);
}
