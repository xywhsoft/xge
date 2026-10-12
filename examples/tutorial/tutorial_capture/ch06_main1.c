/* ch06 — Delta Time：帧率无关的匀速动画（截取一帧） */
#include "tut_capture.h"

static void draw_scene(void)
{
	float y = 300.0f;
	float r = 30.0f;
	int i;

	/* 运动轨迹（虚线效果：多个淡色圆） */
	for ( i = 0; i < 8; i++ ) {
		float x = 100.0f + i * 80.0f;
		uint32_t alpha = (uint32_t)(40 + i * 20);
		xgeShapeCircleFill(x, y, r * 0.6f, XGE_COLOR_RGBA(255, 107, 94, alpha));
	}

	/* 当前球体位置（模拟 t=1.5s 时 x=300） */
	xgeShapeCircleFill(400, y, r, XGE_COLOR_RGBA(255, 107, 94, 255));

	/* 运动方向箭头 */
	xgeShapeLine(440, y, 520, y, 2.0f, XGE_COLOR_RGBA(255, 180, 84, 200));
	xgeShapeLine(520, y, 505, y - 8, 2.0f, XGE_COLOR_RGBA(255, 180, 84, 200));
	xgeShapeLine(520, y, 505, y + 8, 2.0f, XGE_COLOR_RGBA(255, 180, 84, 200));

	/* 基准线 */
	xgeShapeLine(60, y + 60, 740, y + 60, 1.0f, XGE_COLOR_RGBA(100, 120, 140, 100));
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch06", argc, argv);
}
