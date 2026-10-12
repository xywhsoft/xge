/* ch36 — 路径构建：直线段 */
#include "tut_capture.h"

static void draw_scene(void)
{
	xge_shape_ex pShape = NULL;

	/* 三角形路径：MoveTo → LineTo → LineTo → Close */
	xgeShapeExCreate(&pShape);
	xgeShapeExMoveTo(pShape, 200, 80);
	xgeShapeExLineTo(pShape, 320, 280);
	xgeShapeExLineTo(pShape, 80, 280);
	xgeShapeExClose(pShape);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(79, 216, 194, 180));
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 开放折线（不 Close）：描边 */
	xgeShapeExCreate(&pShape);
	xgeShapeExMoveTo(pShape, 400, 100);
	xgeShapeExLineTo(pShape, 500, 200);
	xgeShapeExLineTo(pShape, 450, 300);
	xgeShapeExLineTo(pShape, 600, 350);
	xgeShapeExLineTo(pShape, 700, 250);
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(255, 180, 84, 255));
	xgeShapeExStrokeWidth(pShape, 3.0f);
	xgeShapeExStrokeCap(pShape, XGE_SHAPE_EX_CAP_ROUND);
	xgeShapeExStrokeJoin(pShape, XGE_SHAPE_EX_JOIN_ROUND);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 锯齿形填充 */
	xgeShapeExCreate(&pShape);
	xgeShapeExMoveTo(pShape, 100, 400);
	xgeShapeExLineTo(pShape, 150, 350);
	xgeShapeExLineTo(pShape, 200, 400);
	xgeShapeExLineTo(pShape, 250, 350);
	xgeShapeExLineTo(pShape, 300, 400);
	xgeShapeExLineTo(pShape, 350, 350);
	xgeShapeExLineTo(pShape, 400, 400);
	xgeShapeExLineTo(pShape, 400, 450);
	xgeShapeExLineTo(pShape, 100, 450);
	xgeShapeExClose(pShape);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(255, 107, 94, 160));
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);

	/* 箭头路径 */
	xgeShapeExCreate(&pShape);
	xgeShapeExMoveTo(pShape, 500, 450);
	xgeShapeExLineTo(pShape, 650, 450);
	xgeShapeExLineTo(pShape, 650, 420);
	xgeShapeExLineTo(pShape, 720, 470);
	xgeShapeExLineTo(pShape, 650, 520);
	xgeShapeExLineTo(pShape, 650, 490);
	xgeShapeExLineTo(pShape, 500, 490);
	xgeShapeExClose(pShape);
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(160, 200, 255, 220));
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);
}

int main(int argc, char** argv)
{
	return tut_run(draw_scene, "ch36", argc, argv);
}
