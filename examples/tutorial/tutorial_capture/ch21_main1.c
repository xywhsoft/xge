/* ch21 — 点与线段 */
#include "tut_capture.h"
#include <math.h>

static void draw_scene(void)
{
	int i;
	float cx = 400.0f, cy = 350.0f, r = 130.0f;

	/* 一排渐变大小的点 */
	for ( i = 0; i < 20; i++ ) {
		float x = 100.0f + i * 30.0f;
		float size = 2.0f + i * 0.5f;
		xgeShapePoint(x, 120, size, XGE_COLOR_RGBA(255, 180, 84, 255));
	}

	/* 坐标轴 */
	xgeShapeLine(400, 180, 400, 550, 1.0f, XGE_COLOR_RGBA(109, 179, 242, 150));
	xgeShapeLine(100, 350, 700, 350, 1.0f, XGE_COLOR_RGBA(109, 179, 242, 150));

	/* 星形连线 */
	for ( i = 0; i < 5; i++ ) {
		float a0 = i * 4.0f * 3.14159265f / 5.0f - 3.14159265f / 2.0f;
		float a1 = (i + 1) * 4.0f * 3.14159265f / 5.0f - 3.14159265f / 2.0f;
		xgeShapeLine(
			cx + r * (float)cos(a0), cy + r * (float)sin(a0),
			cx + r * (float)cos(a1), cy + r * (float)sin(a1),
			2.0f, XGE_COLOR_RGBA(255, 180, 84, 255));
	}

	/* 对角线 */
	xgeShapeLine(100, 100, 700, 500, 3.0f, XGE_COLOR_RGBA(255, 107, 94, 100));
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch21", argc, argv);
}
