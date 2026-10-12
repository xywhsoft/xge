/* ch15 — 透视投影：2.5D 透视效果 */
#include "tut_capture.h"
#include <math.h>

static void draw_scene(void)
{
	xge_rect_t rc;
	int i;

	/* 用梯形模拟透视地面网格 */
	/* 远处窄，近处宽 */
	for ( i = 0; i < 8; i++ ) {
		float t = (float)i / 8.0f;
		float y = 150.0f + t * 350.0f;
		float halfW = 50.0f + t * 350.0f;
		float cx = 400.0f;

		/* 横线 */
		xgeShapeLine(cx - halfW, y, cx + halfW, y, 1.0f,
			XGE_COLOR_RGBA(79, 216, 194, (uint32_t)(80 + t * 120)));
	}

	/* 纵向汇聚线（灭点在中心上方） */
	for ( i = -3; i <= 3; i++ ) {
		float topX = 400.0f + i * 15.0f;
		float botX = 400.0f + i * 120.0f;
		xgeShapeLine(topX, 150, botX, 500, 1.0f,
			XGE_COLOR_RGBA(79, 216, 194, 100));
	}

	/* 灭点标记 */
	xgeShapeCircleFill(400, 150, 5, XGE_COLOR_RGBA(255, 180, 84, 255));

	/* 透视缩放的物体（远小近大） */
	xgeShapeCircleFill(350, 200, 10, XGE_COLOR_RGBA(255, 107, 94, 200));
	xgeShapeCircleFill(300, 350, 25, XGE_COLOR_RGBA(255, 107, 94, 220));
	xgeShapeCircleFill(250, 470, 40, XGE_COLOR_RGBA(255, 107, 94, 255));
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch15", argc, argv);
}
