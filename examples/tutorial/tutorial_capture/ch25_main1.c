/* ch25 — 三角形：填充、描边、锯齿装饰 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_vec2_t A, B, C;
	int i;

	/* 大三角形（填充 + 描边） */
	A.fX = 400; A.fY = 80;
	B.fX = 250; B.fY = 380;
	C.fX = 550; C.fY = 380;
	xgeShapeTriangleFill(A, B, C, XGE_COLOR_RGBA(79, 216, 194, 255));
	xgeShapeTriangleStroke(A, B, C, 3.0f, XGE_COLOR_RGBA(255, 180, 84, 255));

	/* 箭头指示器 */
	A.fX = 650; A.fY = 200;
	B.fX = 610; B.fY = 180;
	C.fX = 610; C.fY = 220;
	xgeShapeTriangleFill(A, B, C, XGE_COLOR_RGBA(255, 107, 94, 255));

	/* 锯齿装饰 */
	for ( i = 0; i < 20; i++ ) {
		float x0 = (float)(i * 40);
		uint32_t col = (i % 2) ?
			XGE_COLOR_RGBA(79, 216, 194, 96) :
			XGE_COLOR_RGBA(255, 180, 84, 96);
		A.fX = x0; A.fY = 520;
		B.fX = x0 + 40; B.fY = 520;
		C.fX = x0 + 20; C.fY = 490;
		xgeShapeTriangleFill(A, B, C, col);
	}
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch25", argc, argv);
}
