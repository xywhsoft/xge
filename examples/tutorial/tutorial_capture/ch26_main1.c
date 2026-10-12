/* ch26 — 椭圆：行星轨道 */
#include "tut_capture.h"
#include <math.h>

static void draw_scene(void)
{
	float cx = 400.0f, cy = 300.0f;
	float angle = 0.8f;

	/* 横向椭圆 */
	xgeShapeEllipseFill(200, 150, 120, 60, XGE_COLOR_RGBA(79, 216, 194, 200));

	/* 纵向椭圆 */
	xgeShapeEllipseFill(600, 150, 40, 100, XGE_COLOR_RGBA(255, 180, 84, 200));

	/* 轨道 */
	xgeShapeEllipseStroke(cx, cy, 250, 120, 1.5f, XGE_COLOR_RGBA(100, 120, 140, 150));

	/* 中心恒星 */
	xgeShapeCircleFill(cx, cy, 25, XGE_COLOR_RGBA(255, 180, 84, 255));

	/* 行星 */
	xgeShapeCircleFill(cx + 250.0f * (float)cos(angle),
		cy + 120.0f * (float)sin(angle), 12, XGE_COLOR_RGBA(109, 179, 242, 255));

	/* 角色阴影 */
	xgeShapeEllipseFill(400, 520, 40, 12, XGE_COLOR_RGBA(0, 0, 0, 96));
	xgeShapeCircleFill(400, 460, 30, XGE_COLOR_RGBA(255, 107, 94, 255));
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch26", argc, argv);
}
