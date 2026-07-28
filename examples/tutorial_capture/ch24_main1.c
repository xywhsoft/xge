/* ch24 — 弧线：进度环 + 加载动画 */
#include "tut_capture.h"

#ifndef XGE_PI
#define XGE_PI 3.14159265358979323846f
#endif

static void draw_scene(void)
{
	float offset, fEnd;

	/* 半圆弧 */
	xgeShapeArc(200, 150, 80.0f, XGE_PI, 2.0f * XGE_PI, 4.0f,
		XGE_COLOR_RGBA(79, 216, 194, 255));

	/* 四分之一弧 */
	xgeShapeArc(200, 150, 50.0f, 0, XGE_PI / 2.0f, 3.0f,
		XGE_COLOR_RGBA(255, 180, 84, 255));

	/* 加载动画（270° 弧） */
	xgeShapeArc(550, 150, 50.0f, 0.5f, 0.5f + XGE_PI * 1.5f, 6.0f,
		XGE_COLOR_RGBA(109, 179, 242, 255));

	/* 进度环 72% */
	offset = -XGE_PI / 2.0f;
	fEnd = 0.72f * 2.0f * XGE_PI;
	/* 背景环 */
	xgeShapeArc(400, 400, 80.0f, 0, 2.0f * XGE_PI, 10.0f,
		XGE_COLOR_RGBA(51, 51, 51, 255));
	/* 进度弧 */
	xgeShapeArc(400, 400, 80.0f, offset, offset + fEnd, 10.0f,
		XGE_COLOR_RGBA(79, 216, 194, 255));
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch24", argc, argv);
}
