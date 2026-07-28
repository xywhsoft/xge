/* ch03 — 帧循环：旋转图形（静态截取一帧） */
#include "tut_capture.h"
#include <math.h>

static void draw_scene(void)
{
	float cx = 400.0f, cy = 300.0f;
	float angle = 45.0f * 3.14159265f / 180.0f; /* 模拟旋转 45° */
	float r = 120.0f;
	int i;

	/* 绘制旋转正方形的 4 条边 */
	for ( i = 0; i < 4; i++ ) {
		float a0 = angle + i * 3.14159265f / 2.0f;
		float a1 = angle + (i + 1) * 3.14159265f / 2.0f;
		float x0 = cx + r * (float)cos(a0);
		float y0 = cy + r * (float)sin(a0);
		float x1 = cx + r * (float)cos(a1);
		float y1 = cy + r * (float)sin(a1);
		xgeShapeLine(x0, y0, x1, y1, 3.0f, XGE_COLOR_RGBA(79, 216, 194, 255));
	}

	/* 中心点 */
	xgeShapeCircleFill(cx, cy, 6, XGE_COLOR_RGBA(255, 180, 84, 255));

	/* 旋转轴指示线 */
	xgeShapeLine(cx, cy, cx + r * (float)cos(angle), cy + r * (float)sin(angle),
		1.5f, XGE_COLOR_RGBA(255, 107, 94, 200));

	/* 轨迹圆 */
	xgeShapeCircleStroke(cx, cy, r, 1.0f, XGE_COLOR_RGBA(100, 120, 140, 120));
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch03", argc, argv);
}
