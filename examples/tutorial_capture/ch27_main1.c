/* ch27 — 扇形与弓形：饼图 + 冷却遮罩 */
#include "tut_capture.h"

#ifndef XGE_PI
#define XGE_PI 3.14159265358979323846f
#endif

static void draw_scene(void)
{
	float data[] = { 0.35f, 0.25f, 0.20f, 0.20f };
	uint32_t colors[] = {
		XGE_COLOR_RGBA(79, 216, 194, 255),
		XGE_COLOR_RGBA(255, 180, 84, 255),
		XGE_COLOR_RGBA(255, 107, 94, 255),
		XGE_COLOR_RGBA(109, 179, 242, 255)
	};
	float start = -XGE_PI / 2.0f;
	int i;

	/* 饼图 */
	for ( i = 0; i < 4; i++ ) {
		float end = start + data[i] * 2.0f * XGE_PI;
		xgeShapePieFill(250, 280, 140, 140, start, end, colors[i]);
		start = end;
	}

	/* 技能冷却遮罩 */
	xgeShapeCircleFill(600, 200, 40, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeShapePieFill(600, 200, 40, 40,
		-XGE_PI / 2.0f, -XGE_PI / 2.0f + 0.7f * 2.0f * XGE_PI,
		XGE_COLOR_RGBA(0, 0, 0, 160));

	/* 弓形 */
	xgeShapeChordFill(600, 430, 100, 100, 0, XGE_PI,
		XGE_COLOR_RGBA(255, 180, 84, 128));
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch27", argc, argv);
}
