/* ch23 — 圆形：填充、描边、同心靶心 */
#include "tut_capture.h"

static void draw_scene(void)
{
	int i;

	/* 实心圆 */
	xgeShapeCircleFill(200, 200, 80, XGE_COLOR_RGBA(79, 216, 194, 255));

	/* 空心圆环 */
	xgeShapeCircleStroke(200, 200, 110, 4.0f, XGE_COLOR_RGBA(255, 180, 84, 255));

	/* 同心圆靶心 */
	for ( i = 5; i >= 1; i-- ) {
		uint32_t c = (i % 2) ?
			XGE_COLOR_RGBA(255, 107, 94, 255) :
			XGE_COLOR_RGBA(255, 255, 255, 255);
		xgeShapeCircleFill(550, 300, i * 25.0f, c);
	}

	/* 多个不同大小的圆 */
	xgeShapeCircleFill(150, 450, 30, XGE_COLOR_RGBA(109, 179, 242, 200));
	xgeShapeCircleFill(250, 480, 20, XGE_COLOR_RGBA(255, 180, 84, 200));
	xgeShapeCircleFill(350, 460, 40, XGE_COLOR_RGBA(79, 216, 194, 150));
	xgeShapeCircleStroke(500, 480, 35, 2.0f, XGE_COLOR_RGBA(255, 107, 94, 200));
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch23", argc, argv);
}
